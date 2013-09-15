
#include <boost/bind.hpp>

#include "DataMessage.hpp"
#include "JoinMessage.hpp"
#include "JoinNotifMessage.hpp"
#include "JoinAckMessage.hpp"
#include "ReadyMessage.hpp"
#include "ReadyNotifMessage.hpp"

#include "Network.hpp"

Network::Network(shared_ptr<asio::io_service> _ios,
                 shared_ptr<tcp::resolver> _resolver,
                 shared_ptr<Peer> p)
  : io_service(_ios), resolver(_resolver), local_peer(p), join_token(true),
    h_logs( logs.get<LOG_INDEX_HASH>() ), t_logs( logs.get<LOG_INDEX_TIME>() )
{
}

void Network::add_new_peer(shared_ptr<Peer> p)
{
  new_peers[ p->get_address() ] = pair<shared_ptr<Peer>, unsigned short>(p, 0);

  Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                      asio::placeholders::error,
                                      asio::placeholders::bytes_transferred, p);
  p->start_listening(listen_handler);
}

void Network::join(string entry_ip, string entry_port)
{
  // TODO: check if not already CONNECTED
  try {
    shared_ptr<Peer> entry_peer = connect_peer(entry_ip, entry_port);
    local_peer->set_state(PEER_STATE_JOINING);

    add_new_peer(entry_peer);
  
    Message* join = new JoinMessage(local_peer->get_id(), local_peer->get_key(),
                                    settings.LISTEN_PORT);
    send( join, entry_peer );
    delete join;
  }
  catch (std::exception& e) {
    DEBUG("Network::join: " << e.what());
  }
}

shared_ptr<Peer> Network::connect_peer(string ip, string port)
{
  tcp::resolver::query query(ip, port);
  tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

  shared_ptr<tcp::socket> socket(new tcp::socket(*io_service));
  asio::connect(*socket, endpoint_iterator);

  shared_ptr<Peer> new_peer(new Peer(socket));

  // DEBUG("Connected to entry point at "
  //       << new_peer->get_socket().local_endpoint().address().to_string()
  //       << ":" << new_peer->get_socket().local_endpoint().port() );
        
  return new_peer;
}

void Network::broadcast(Message* message, bool add_stamp)
{
  if (add_stamp)
    message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());
  
  PeerMap succs = successors[local_group->get_id()];
  PeerMap::iterator it;
  for (it=succs.begin(); it!=succs.end(); it++) {
    it->second->send(msg);
  }
  log_message( message, local_peer );
}

void Network::broadcast_data(string content)
{
  DataMessage* data = new DataMessage(content);
  broadcast(data, true);
  delete data;
}

void Network::send_all(string message)
{
  DEBUG("\tSending cleartext to all peers...");
  PeerMap::iterator it;
  for (it=peers.begin(); it!=peers.end(); it++) {

    it->second->send(message);
  }
}

void Network::send(Message* message, shared_ptr<Peer> peer)
{
  message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());

  peer->send(msg);
  log_message(message, local_peer);
}

void Network::send_ready(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    Message* ready = new ReadyMessage();
    send(ready, peer);
    delete ready;

    ready_timers.erase( peer->get_id() );
  }
  else {
    DEBUG("Network::send_ready: " << error.message());
  }
}

void Network::answer_join_request(shared_ptr<Peer> peer, unsigned short port)
{
  join_token = false;
  Message* notif = new JoinNotifMessage( local_group->get_id(), peer->get_id(),
                                         peer->get_key(), peer->get_address(), port );

  broadcast(notif, true);
  delete notif;

  new_peers.erase( peer->get_address() );
  handle_join(peer);

  shared_ptr<asio::deadline_timer> t
    (new asio::deadline_timer(*io_service, posix_time::seconds(READY_TIME)));
  ready_timers[ peer->get_id() ] = t;

  t->async_wait( bind(&Network::send_ready, this,
                      asio::placeholders::error, peer) );
  
}

void Network::check_for_new_peers()
{
  JoinMap::iterator it = new_peers.begin();

  while ( it!=new_peers.end() && !(it->second.first->is_known()) )
    it++;

  if (it!=new_peers.end())
    answer_join_request( it->second.first, it->second.second );
  else
    join_token = true;

}

void Network::complete_join(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    peer->set_state(PEER_STATE_CONNECTED);
    join_timers.erase( peer->get_id() );
    
    check_for_new_peers();

  }
  else {
    DEBUG("Network::complete_join: " << error.message());
  }
}

void Network::handle_disconnect(shared_ptr<Peer> p)
{
   DEBUG("Peer @" << p->get_address() << " disconnected.");
    if ( peers.erase( p->get_id() ) ) {

      bool found(false);
      map<string, shared_ptr<Group> >::iterator it = groups.begin();

      while (!found && it!=groups.end()) {

        try {
          it->second->remove_peer(p);
          found = true;
        }
        catch (PeerNotFoundException& e) {
          it++;
        }
      }

      if (found)
        it->second->update_neighbours(predecessors[ it->first ],
                                      successors[ it->first ],
                                      local_peer);
    }
    else {
      new_peers.erase( p->get_address() );
    }
}

void Network::handle_incoming_message(const system::error_code& error,
                                      size_t bytes_transferred,
                                      shared_ptr<Peer> emitter)
{
  if (error == asio::error::eof) 
    handle_disconnect(emitter);

  else if (error) {
    DEBUG("Network::handle_incoming_message: " << error.message());
  }

  else {
    string received_message = emitter->get_last_message(bytes_transferred);
    emitter->listen();

    try {
      Message* message = parse_message(received_message);

      LogIndexHash::iterator it = find_log(message);

      if (it != h_logs.end()) {
        h_logs.modify(it, ack_message(emitter));
        delete message;
        // either we sent this message, or we've already received it
        // (and forwarded it), so we shouldn't have anything else to do
        return;
      }
      else  {
        if (emitter->is_known()) 
          log_message(message, emitter);

        if (message->is_broadcast())
          broadcast(message);
      }


      switch (message->type) {

      case MESSAGE_TYPE_JOIN: {
        // if we've been alone so far, consider we constitute a functional network
        if (local_peer->get_state() != PEER_STATE_CONNECTED) {
          local_peer->set_state(PEER_STATE_CONNECTED);

          string concat( local_peer->get_id() + itos(milliseconds_since_epoch()) ) ;
          local_group = shared_ptr<Group>(new Group(make_hash(concat)));
          groups.insert( pair<string, shared_ptr<Group> >( local_group->get_id(),
                                                           local_group ) );
          
          local_group->add_peer(local_peer);
          
        }

        // As the entry point for the emitter, we will
        // - broadcast its join request (as a join notif) to the group
        // - move emitter from new_peers to joining_peers
        // - wait for T, then send READY to emitter
        // - add it to correct position in rings

        JoinMessage* msg = dynamic_cast<JoinMessage*>(message);

        emitter->init( msg->id, msg->pub_k );
        emitter->set_state( PEER_STATE_JOINING );
        log_message(message, emitter);

        if (join_token)
          answer_join_request(emitter, msg->port);
        else
          new_peers[ emitter->get_address() ].second = msg->port;

        
      }
        break;

      case MESSAGE_TYPE_JOIN_NOTIF: {
        // Emitter is the entry point for some peer we don't know yet. 
        // - add this peer to our view
        // - send it a join ack so that it knows about us
        // - add it to joining_peers
        // - add it to correct position in rings

        JoinNotifMessage* msg = dynamic_cast<JoinNotifMessage*>(message);
        // TODO: if ID is valid
        join_token = false;
        shared_ptr<Peer> new_peer = connect_peer( msg->ip,
                                                  itos(msg->port) );

        new_peer->init( msg->peer_id, msg->pub_k );
        new_peer->set_state(PEER_STATE_JOINING);

        handle_join(new_peer);        

        Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                            asio::placeholders::error,
                                            asio::placeholders::bytes_transferred,
                                            new_peer);
        new_peer->start_listening(listen_handler);

      }
        break;

      case MESSAGE_TYPE_JOIN_ACK: {

        // We're joining, group members start making themselves known
        // - store the emitter in regular peers map

        JoinAckMessage* msg = dynamic_cast<JoinAckMessage*>(message);
        // check whether we're joining/readying, maybe?
        
        if (groups.find( msg->group_id ) == groups.end()) {
          shared_ptr<Group> new_group = shared_ptr<Group>(new Group(msg->group_id));
          groups.insert( pair<string, shared_ptr<Group> >(msg->group_id, new_group) );

          if (!local_group)
            local_group = new_group;
        }
          

        emitter->init( msg->peer_id, msg->pub_k );
        emitter->set_state(PEER_STATE_CONNECTED);

        new_peers.erase( emitter->get_address() );
        peers[ emitter->get_id() ] = emitter;

        groups[msg->group_id]->add_peer(emitter);

        log_message( message, emitter );
        
      }
        break;

      case MESSAGE_TYPE_READY: {

        // entry point has communicated our status to the group we belong to ;
        // they all have been sending us JOIN_ACK so that now,
        // we can compute our position on the rings

        local_group->add_peer(local_peer);
        local_group->update_neighbours( predecessors[local_group->get_id()],
                                        successors[local_group->get_id()],
                                        local_peer );


        local_peer->set_state(PEER_STATE_CONNECTED);

        Message* notif = new ReadyNotifMessage();

        notif->make_stamp(local_peer->get_id());
        string notif_msg = notif->serialize();

        PeerMap preds = predecessors[local_group->get_id()];
        PeerMap succs = successors[local_group->get_id()];

        PeerMap directs(preds);
        directs.insert( succs.begin(), succs.end() );

        for (PeerMap::iterator it=directs.begin(); it!=directs.end(); it++)
          it->second->send(notif_msg);

        log_message( notif, local_peer );
        
        delete notif;
      }
        break;

      case MESSAGE_TYPE_READY_NOTIF: {

        // emitter is now ready to communicate with us: move to regular peers map

        emitter->set_state(PEER_STATE_CONNECTED);
        check_for_new_peers();
      }
        break;

      case MESSAGE_TYPE_DATA: {

        DataMessage* msg = dynamic_cast<DataMessage*>(message);
        
        cout << "Peer " << ( emitter->is_known() ? emitter->get_id()
                             : string("@" + emitter->get_address()) )
             << " sent this:\n\t" << msg->data << endl;
      }
        break;
        
      default: 
        throw MessageParseException();
      }

      delete message;
    }
    catch (MessageParseException& e) {
      cout << "Couldn't make sense of this:" << endl;
      cout << "\t";

      for (uint n=0; n< (received_message.size()); n++)
        cout << (int) ((unsigned char) received_message[n])
             << (n+1==received_message.size() ? "" : "-");
      cout << endl;
    }
  }  
}

void Network::handle_join(shared_ptr<Peer> peer)
{
  // TODO: send group to this function
  shared_ptr<Group> group = local_group;

  peers[ peer->get_id() ] = peer;

  group->add_peer(peer); 

  PeerMap& preds = predecessors[ group->get_id() ];
  PeerMap& succs = successors[ group->get_id() ];  
  group->update_neighbours(preds, succs, local_peer);

  // - add to rings
  // - send join ack
  // - if direct pred/succ, wait for READY before setting state to CONNECTED
  // - else, wait for 2T before setting to CONNECTED

  Message* ack = new JoinAckMessage( group->get_id(),
                                     local_peer->get_id(), local_peer->get_key() );
  send(ack, peer);
  delete ack;

  if ( preds.find(peer->get_id()) == preds.end()
       && succs.find(peer->get_id()) == succs.end() ) {

    shared_ptr<asio::deadline_timer> t
      (new asio::deadline_timer(*io_service, posix_time::seconds(JOIN_COMPLETE_TIME)));

    join_timers[ peer->get_id() ] = t;
    t->async_wait( bind(&Network::complete_join, this,
                        asio::placeholders::error, peer) );
  }
}

void Network::print_rings()
{
  map<string, shared_ptr<Group> >::iterator g_it;
  for (g_it = groups.begin(); g_it != groups.end(); g_it++) {

    cout << "\tIn group ";
    display_chars(g_it->first, 10);
    cout << " :" << endl;
    g_it->second->display_rings();

    PeerMap preds = predecessors[ g_it->first ];
    PeerMap succs = successors[ g_it->first ];

    cout << "My predecessors are:" << endl;

    for (PeerMap::iterator it=preds.begin(); it!=preds.end(); it++) {
      cout << "- " << it->second->get_id() << endl;
    }
    cout << "My successors are:" << endl;
    for (PeerMap::iterator it=succs.begin(); it!=succs.end(); it++) {
      cout << "- " << it->second->get_id() << endl;
    }

    cout << endl;

  }
}

void Network::print_logs()
{
  for (LogIndexTime::iterator it=t_logs.begin(); it!=t_logs.end(); it++) {
    Message* m = parse_message( it->message );
    m->display();
    cout << "Received from:" << endl;

    map<string, int> preds = it->control;
    for (map<string, int>::iterator jt=preds.begin(); jt!=preds.end(); jt++) {
      cout << "- " << jt->first << ": " << jt->second << endl;
    }
    cout << endl;
    delete m;
  }
}

void Network::log_message(Message* message, shared_ptr<Peer> emitter)
{
  LogIndexHash::iterator it = find_log(message);
  
  if ( it==h_logs.end() ) {
    MessageLog ml;
    ml.message = message->serialize();

    // make a list of peers we expect to receive this message from

    if (message->is_broadcast()) {
      PeerMap preds = predecessors[ local_group->get_id() ];
      for (PeerMap::iterator p=preds.begin(); p!=preds.end(); p++)
        ml.control[ p->second->get_id() ] = 0;
    }
    pair<LogIndexHash::iterator, bool> pair = h_logs.insert( ml );
    if (pair.second)
      it = pair.first;
  }
            
  if ( !emitter->is_local() && it!=h_logs.end() )
    h_logs.modify(it, ack_message(emitter));
}

Network::LogIndexHash::iterator Network::find_log(Message* message) {

  MessageLog ml;
  ml.message = message->serialize();

  return h_logs.find(ml);
}
