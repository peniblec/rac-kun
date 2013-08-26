#include <boost/bind.hpp>

#include "Message.hpp"
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
  : io_service(_ios), resolver(_resolver), local_peer(p)
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
  predecessors.clear();
  successors.clear();

  try {
    for (int i=0; i<RINGS_NB; i++) {
      predecessors.insert( rings[i].get_predecessor( local_peer ) );
      successors.insert( rings[i].get_successor( local_peer ) );    
    }
  }
  catch (PeerNotFoundException& e) {
    cout << e.what() << endl;
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

void Network::send_all(string message)
{
  DEBUG("\tSending cleartext to all peers...");
  PeerMap::iterator it;
  for (it=peers.begin(); it!=peers.end(); it++) {

    it->second->send(message);
  }
}

void Network::send_all(Message* message)
{
  // DEBUG("Sending \"" << message << "\" to " << peers.size() << " peers.");
  message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());

  PeerMap::iterator it;
  for (it=peers.begin(); it!=peers.end(); it++) {

    it->second->send(msg);
  }
  log_message(msg, local_peer);
  delete message;
}

void Network::send(Message* message, shared_ptr<Peer> peer)
{
  message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());

  peer->send(msg);
  log_message(msg, local_peer);
  delete message;
}

void Network::send_ready(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    peer->set_state(PEER_STATE_READYING);
    Message* ready = new ReadyMessage();

    send(ready, peer);
    ready_timers.erase( peer->get_id() );
  }
  else {
    DEBUG("Network::send_ready: " << error.message());
  }
}

void Network::handle_incoming_message(const system::error_code& error,
                                      size_t bytes_transferred,
                                      shared_ptr<Peer> emitter)
{
  if (error == asio::error::eof) {
    DEBUG("Peer @" << emitter->get_address() << " disconnected.");
    if ( peers.erase( emitter->get_id() ) ) {
      // TODO: remove from rings
    }
    else {
      new_peers.erase( emitter->get_address() );
    }
  }
  else if (error) {
    DEBUG("Network::handle_incoming_message: " << error.message());
  }
  else {
    try {
      string received_message = emitter->get_last_message(bytes_transferred);
      Message* message = parse_message(received_message);

      if (emitter->is_known())
        log_message(received_message, emitter);

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
        log_message(received_message, emitter);

        Message* notif = new JoinNotifMessage( msg->get_id(), msg->get_key(), emitter->get_address() );

        send_all(notif);

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

        // if ID is valid
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
        new_peers.erase( emitter->get_address() );
        peers[ emitter->get_id() ] = emitter;

        emitter->set_state(PEER_STATE_CONNECTED);
        
      }
        break;

      case MESSAGE_TYPE_READY: {

        // time to send READY_NOTIF to everyone, and compute our position on the rings
        local_peer->set_state(PEER_STATE_READYING);
        // TODO: figure out whether Readying state is useful
        
        Message* notif = new ReadyNotifMessage();
        send_all(notif); // TODO: send only to direct predecessors/followers

        local_peer->set_state(PEER_STATE_CONNECTED);
        add_peer_to_rings(local_peer);
        for (PeerMap::iterator it = peers.begin(); it!=peers.end(); it++) {

          add_peer_to_rings(it->second);
        }
      }
        break;

      case MESSAGE_TYPE_READY_NOTIF: {

        // emitter is now ready to communicate with us: move to regular peers map

        emitter->set_state(PEER_STATE_CONNECTED);
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
      string buffer = emitter->get_last_message(bytes_transferred);
      for (uint n=0; n< (buffer.size()); n++)
        cout << (int) ((unsigned char) buffer[n]) << (n+1==buffer.size() ? "" : "-");
      cout << endl;
    }
    emitter->listen();
  }  
}

void Network::handle_join(shared_ptr<Peer> peer)
{
  peers[ peer->get_id() ] = peer;
  peer->set_state( PEER_STATE_JOINING);

  add_peer_to_rings(peer);

  // TODO:
  // - add to rings
  // - send join ack
  // - if direct pred/succ, wait for READY before setting state to CONNECTED
  // - else, wait for 2T before setting to CONNECTED

  Message* ack = new JoinAckMessage( local_peer->get_id(), local_peer->get_key() );
  send(ack, peer);
}

void Network::print_rings()
{
  for (int i=0; i<RINGS_NB; i++) {

    rings[i].display();
    cout << endl;
  }

  cout << "My predecessors are:" << endl;
  for (PeerSet::iterator it=predecessors.begin(); it!=predecessors.end(); it++) {
    cout << "- " << (*it)->get_id() << endl;
  }
  cout << "My successors are:" << endl;
  for (PeerSet::iterator it=successors.begin(); it!=successors.end(); it++) {
    cout << "- " << (*it)->get_id() << endl;
  }
}

void Network::broadcast(string msg)
{
  // LOL

}

void Network::print_logs()
{
  cout << "Logs are " << sizeof(logs) << " bytes for "
       << logs.size() << " elements." << endl;
  Message* m = NULL;
  LogIndexTime& index = logs.get<LOG_INDEX_TIME>();

  for (LogIndexTime::iterator it=index.begin(); it!=index.end(); it++) {
    m = parse_message( it->message );
    cout << "Received/sent a " << MessageTypeNames[ m->get_type() ] << endl;
    map<string, int> preds = it->control;
    for (map<string, int>::iterator jt=preds.begin(); jt!=preds.end(); jt++) {
      cout << "- " << jt->first << ": " << jt->second << endl;
    }
  }
  if (m)
    delete m;
}

void Network::log_message(string message, shared_ptr<Peer> emitter)
{
  MessageLog ml;
  ml.message = message;

  LogIndexHash& index = logs.get<LOG_INDEX_HASH>();
  LogIndexHash::iterator it = index.find(ml);

  if ( it==index.end() ) {
    // initialize predecessors

    for (PeerSet::iterator p=predecessors.begin(); p!=predecessors.end(); p++)
      ml.control[ (*p)->get_id() ] = 0;

    pair<LogIndexHash::iterator, bool> pair = index.insert( ml );
    if (pair.second)
      it = pair.first;
  }
  if ( !emitter->is_local() && it!=index.end() )
    index.modify(it, ack_message(emitter));
}


