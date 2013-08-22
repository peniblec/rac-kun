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
                                      asio::placeholders::error, p);
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
  entry_peer->set_state(PEER_STATE_CONNECTED);
  local_peer->set_state(PEER_STATE_JOINING);

  add_new_peer(entry_peer);
  
  JoinMessage join(local_peer->get_id(), local_peer->get_key());
  join.make_stamp( local_peer->get_id() );
  entry_peer->send(join.serialize());
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
  DEBUG("Sending \"" << message << "\" to " << peers.size() << " peers.");

  PeerMap::iterator it;
  for (it=peers.begin(); it!=peers.end(); it++) {

    it->second->send(message);
  }
}

void Network::send_ready(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    peer->set_state(PEER_STATE_READYING);
    ReadyMessage ready;
    ready.make_stamp( local_peer->get_id() );
    peer->send(ready.serialize());
    ready_timers.erase( peer->get_id() );
  }
  else {
    DEBUG("Network::send_ready: " << error.message());
  }
}

void Network::handle_incoming_message(const system::error_code& error,
                                      shared_ptr<Peer> emitter)
{
  if (error == asio::error::eof) {
    DEBUG("Peer @" << emitter->get_address() << " disconnected.");
    if ( !new_peers.erase( emitter->get_address() ) ) {
      peers.erase( emitter->get_id() );
      // TODO: remove from rings
    }
  }
  else if (error) {
    DEBUG("Network::handle_incoming_message: " << error.message());
  }
  else {
    try {
      Message* message = parse_message(emitter->get_last_message());

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

        JoinNotifMessage notif( msg->get_id(), msg->get_key(), emitter->get_address() );
        notif.make_stamp( local_peer->get_id() );

        send_all(notif.serialize());

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
                                            asio::placeholders::error, new_peer);
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
        
        ReadyNotifMessage notif;
        notif.make_stamp( local_peer->get_id() );
        send_all(notif.serialize()); // TODO: send only to direct predecessors/followers

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

      DEBUG("Peer " << emitter->get_id() << " sent a "
            << MessageTypeNames[ message->get_type() ] << ":");
      message->display();
      
      delete message;
    }
    catch (MessageParseException& e) {
      DEBUG("Couldn't make sense of this: " << emitter->get_last_message());
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

  JoinAckMessage ack( local_peer->get_id(), local_peer->get_key() );
  ack.make_stamp( local_peer->get_id() );
  peer->send( ack.serialize() );
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
