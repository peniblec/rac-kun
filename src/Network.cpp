#include <boost/bind.hpp>

#include "DataMessage.hpp"
#include "JoinMessage.hpp"
#include "JoinNotifMessage.hpp"
#include "JoinAckMessage.hpp"
#include "ReadyMessage.hpp"
#include "ReadyNotifMessage.hpp"

#include "Network.hpp"
#include "Utils.hpp"

Network::Network(shared_ptr<asio::io_service> _ios,
                 shared_ptr<tcp::resolver> _resolver,
                 shared_ptr<Peer> p)
  : io_service(_ios), resolver(_resolver), local_peer(p),
    h_logs( logs.get<LOG_INDEX_HASH>() ), t_logs( logs.get<LOG_INDEX_TIME>() )
{
  for (int i=0; i<RINGS_NB; i++)
    rings[i] = Ring(i);
}

void Network::add_new_peer(shared_ptr<Peer> p)
{
  new_peers[ p->get_address() ] = p;

  Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                      asio::placeholders::error,
                                      asio::placeholders::bytes_transferred, p);
  p->start_listening(listen_handler);
}

void Network::add_peer_to_rings(shared_ptr<Peer> p)
{
  for (int i=0; i<RINGS_NB; i++) {

    rings[i].add_peer(p);
  }
  update_my_neighbours();
}

void Network::update_my_neighbours()
{
  PeerMap preds, succs;
  shared_ptr<Peer> p;
  try {
    for (int i=0; i<RINGS_NB; i++) {
    
      p = rings[i].get_predecessor( local_peer );
      preds.insert( pair<string, shared_ptr<Peer> >( p->get_id(), p ) );

      p = rings[i].get_successor( local_peer );
      succs.insert( pair<string, shared_ptr<Peer> >( p->get_id(), p ) );
    }
    predecessors = preds;
    successors = succs;
  }
  catch (PeerNotFoundException& e) { 
    DEBUG("Network::update_my_neighbours: " << e.what());
    predecessors.clear();
    successors.clear();
  }

}

void Network::join(string entry_point)
{
  // TODO: check if not already CONNECTED
  shared_ptr<Peer> entry_peer = connect_peer(entry_point);
  local_peer->set_state(PEER_STATE_JOINING);

  add_new_peer(entry_peer);
  
  Message* join = new JoinMessage(local_peer->get_id(), local_peer->get_key());
  send( join, entry_peer );
  delete join;
}

shared_ptr<Peer> Network::connect_peer(string peer_name)
{
  tcp::resolver::query query(peer_name, itos(RAC_PORT));
  tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

  shared_ptr<tcp::socket> socket(new tcp::socket(*io_service));
  asio::connect(*socket, endpoint_iterator);

  shared_ptr<Peer> new_peer(new Peer(socket));


  return new_peer;
}

void Network::broadcast(Message* message, bool add_stamp)
{
  if (add_stamp)
    message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());
  
  PeerMap::iterator it;
  for (it=successors.begin(); it!=successors.end(); it++) {
    it->second->send(msg);
  }
  log_message( message, local_peer );
}

void Network::broadcast_data(string message)
{
  DataMessage* data = new DataMessage(message);
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

    peer->set_state(PEER_STATE_READYING);
    ready_timers.erase( peer->get_id() );
  }
  else {
    DEBUG("Network::send_ready: " << error.message());
  }
}

void Network::complete_join(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    peer->set_state(PEER_STATE_CONNECTED);
    join_timers.erase( peer->get_id() );
  }
  else {
    DEBUG("Network::complete_join: " << error.message());
  }
}

void Network::handle_disconnect(shared_ptr<Peer> p)
{
   DEBUG("Peer @" << p->get_address() << " disconnected.");
    if ( peers.erase( p->get_id() ) ) {

      for (int i=0; i<RINGS_NB; i++) {
        rings[i].remove_peer(p);
      }
      update_my_neighbours();
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


      switch (message->get_type()) {

      case MESSAGE_TYPE_JOIN: {
        // if we've been alone so far, consider we constitute a functional network
        if (local_peer->get_state() != PEER_STATE_CONNECTED) {
          local_peer->set_state(PEER_STATE_CONNECTED);
          add_peer_to_rings(local_peer);
        }

        // As the entry point for the emitter, we will
        // - broadcast its join request (as a join notif) to the group
        // - move emitter from new_peers to joining_peers
        // - wait for T, then send READY to emitter
        // - add it to correct position in rings
        
        JoinMessage* msg = dynamic_cast<JoinMessage*>(message);

        emitter->init( msg->get_id(), msg->get_key() );
        log_message(message, emitter);

        Message* notif = new JoinNotifMessage( msg->get_id(), msg->get_key(),
                                               emitter->get_address() );

        broadcast(notif, true);
        delete notif;

        new_peers.erase( emitter->get_address() );
        handle_join(emitter);

        shared_ptr<asio::deadline_timer> t
          (new asio::deadline_timer(*io_service, posix_time::seconds(READY_TIME)));
        ready_timers[ emitter->get_id() ] = t;

        t->async_wait( bind(&Network::send_ready, this,
                            asio::placeholders::error, emitter) );
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
        shared_ptr<Peer> new_peer = connect_peer( msg->get_ip() );
        new_peer->init( msg->get_id(), msg->get_key() );

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
        
        emitter->init( msg->get_id(), msg->get_key() );
        emitter->set_state(PEER_STATE_CONNECTED);
        new_peers.erase( emitter->get_address() );
        peers[ emitter->get_id() ] = emitter;

        log_message( message, emitter );
        
      }
        break;

      case MESSAGE_TYPE_READY: {

        // entry point has communicated our status to the group we belong to ;
        // they all have been sending us JOIN_ACK so that now,
        // we can compute our position on the rings

        // local_peer->set_state(PEER_STATE_READYING); // TODO: is this state useful?
        
        add_peer_to_rings(local_peer);
        for (PeerMap::iterator it = peers.begin(); it!=peers.end(); it++) {
          add_peer_to_rings(it->second);
        }

        local_peer->set_state(PEER_STATE_CONNECTED);

        Message* notif = new ReadyNotifMessage();

        notif->make_stamp(local_peer->get_id());
        string notif_msg = notif->serialize();

        PeerMap directs(predecessors);
        directs.insert( successors.begin(), successors.end() );
        for (PeerMap::iterator it=directs.begin(); it!=directs.end(); it++)
          it->second->send(notif_msg);

        log_message( notif, local_peer );
        
        delete notif;
      }
        break;

      case MESSAGE_TYPE_READY_NOTIF: {

        // emitter is now ready to communicate with us: move to regular peers map

        emitter->set_state(PEER_STATE_CONNECTED);
      }
        break;

      case MESSAGE_TYPE_DATA: {

        DataMessage* msg = dynamic_cast<DataMessage*>(message);
        
        cout << "Peer " << ( emitter->is_known() ? emitter->get_id()
                             : string("@" + emitter->get_address()) )
             << " sent this:\n\t" << msg->get_data() << endl;
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
  peers[ peer->get_id() ] = peer;
  peer->set_state( PEER_STATE_JOINING);

  add_peer_to_rings(peer);

  // - add to rings
  // - send join ack
  // - if direct pred/succ, wait for READY before setting state to CONNECTED
  // - else, wait for 2T before setting to CONNECTED

  Message* ack = new JoinAckMessage( local_peer->get_id(), local_peer->get_key() );
  send(ack, peer);
  delete ack;

  if ( predecessors.find(peer->get_id()) == predecessors.end()
       && successors.find(peer->get_id()) == successors.end() ) {

    shared_ptr<asio::deadline_timer> t
      (new asio::deadline_timer(*io_service, posix_time::seconds(JOIN_COMPLETE_TIME)));

    join_timers[ peer->get_id() ] = t;
    t->async_wait( bind(&Network::complete_join, this,
                        asio::placeholders::error, peer) );
  }
}

void Network::print_rings()
{
  for (int i=0; i<RINGS_NB; i++) {

    rings[i].display();
    cout << endl;
  }

  cout << "My predecessors are:" << endl;
  for (PeerMap::iterator it=predecessors.begin(); it!=predecessors.end(); it++) {
    cout << "- " << it->second->get_id() << endl;
  }
  cout << "My successors are:" << endl;
  for (PeerMap::iterator it=successors.begin(); it!=successors.end(); it++) {
    cout << "- " << it->second->get_id() << endl;
  }
}

void Network::print_logs()
{
  cout << "Logs are " << sizeof(logs) << " bytes for "
       << logs.size() << " elements." << endl;

  for (LogIndexTime::iterator it=t_logs.begin(); it!=t_logs.end(); it++) {
    Message* m = parse_message( it->message );

    cout << "Received/sent a " << MessageTypeNames[ m->get_type() ] << endl;
    
    map<string, int> preds = it->control;
    for (map<string, int>::iterator jt=preds.begin(); jt!=preds.end(); jt++) {
      cout << "- " << jt->first << ": " << jt->second << endl;
    }
    delete m;
  }
}

void Network::log_message(Message* message, shared_ptr<Peer> emitter)
{
  LogIndexHash::iterator it = find_log(message);
  
  if ( it==h_logs.end() ) {// TODO: is this check useful or just plainly redundant?
    MessageLog ml;
    ml.message = message->serialize();

    // make a list of peers we expect to receive this message from

    if (message->is_broadcast())
      for (PeerMap::iterator p=predecessors.begin(); p!=predecessors.end(); p++)
        ml.control[ p->second->get_id() ] = 0;

    pair<LogIndexHash::iterator, bool> pair = h_logs.insert( ml );
    if (pair.second)
      it = pair.first;
  }
            
  if ( /*!emitter->is_local() &&*/ it!=h_logs.end() )
    h_logs.modify(it, ack_message(emitter));
}

Network::LogIndexHash::iterator Network::find_log(Message* message) {

  MessageLog ml;
  ml.message = message->serialize();

  return h_logs.find(ml);
}
