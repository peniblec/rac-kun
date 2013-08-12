#include <boost/bind.hpp>

#include "Message.hpp"
#include "JoinNotifMessage.hpp"
#include "Network.hpp"
#include "Utils.hpp"

Network::Network(shared_ptr<asio::io_service> _ios,
                 shared_ptr<tcp::resolver> _resolver,
                 LocalPeer& p)
  : io_service(_ios), resolver(_resolver), local_peer(p)
{

}

void Network::add_new_peer(shared_ptr<Peer> p, PeerMap& some_map)
{
  some_map[ p->get_address() ] = p;

  Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                      asio::placeholders::error, p);
  p->start_listening(listen_handler);
}

shared_ptr<Peer> Network::join(string entry_point)
{
  shared_ptr<Peer> entry_peer = connect_peer(entry_point);
  entry_peer->set_state(PEER_STATE_CONNECTED);

  add_new_peer(entry_peer, peers);
  
  return entry_peer;
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

void Network::handle_incoming_message(const system::error_code& error,
                                      shared_ptr<Peer> emitter)
{
  if (error == asio::error::eof) {
    DEBUG("Peer @" << emitter->get_address() << " disconnected.");
    new_peers.erase( emitter->get_id() );
  }
  else if (error) {
    DEBUG("Network::handle_incoming_message: " << error.message());
  }
  else {
    try {
      Message* message = parse_message(emitter->get_last_message());
      message->display();

      switch (message->get_type()) {

      case MESSAGE_TYPE_JOIN: {
        // if we've been alone so far, consider we constitute a functional network
        if (local_peer.get_state() != LOCAL_STATE_CONNECTED)
          local_peer.set_state(LOCAL_STATE_CONNECTED);

        JoinMessage* msg = dynamic_cast<JoinMessage*>(message);

        emitter->init( msg->get_id(), msg->get_key() );

        JoinNotifMessage notif( msg->get_id(), msg->get_key(), emitter->get_address() );

        send_all(notif.serialize());

        joining_peers[ emitter->get_id() ] = emitter;
        emitter->set_state(PEER_STATE_JOINING);
        new_peers.erase( emitter->get_address() );
        
        
        // wait for T, then send READY to emitter
      }
        break;

      case MESSAGE_TYPE_JOIN_NOTIF: {

        JoinNotifMessage* msg = dynamic_cast<JoinNotifMessage*>(message);

        // if ID is valid
        shared_ptr<Peer> new_peer = connect_peer( msg->get_ip() );
        new_peer->set_state(PEER_STATE_JOINING);
        add_new_peer(new_peer, joining_peers);
      }
        break;
        
      default: {
        // bleh
      }

      }
      message->display();

    }
    catch (MessageParseException& e) {
      DEBUG("Couldn't make sense of this: " << emitter->get_last_message());
    }
    emitter->listen();
  }  
}



// void Network::check_peers()
// {
//   for (uint p=0 ; p<peers.size() ; p++) {

//     while (peers.size() > p  && ! peers[p]->is_alive() ) {
//       peers.erase( peers.begin()+p );
//       DEBUG("Removed peer, " << peers.size() << " remaining.");
//     }
//   }
// }
