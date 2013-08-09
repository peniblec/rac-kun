#include <boost/bind.hpp>

#include "Message.hpp"
#include "Network.hpp"
#include "Utils.hpp"

Network::Network(shared_ptr<asio::io_service> _ios,
                 shared_ptr<tcp::resolver> _resolver,
                 LocalPeer& p)
  : io_service(_ios), resolver(_resolver), local_peer(p)
{

}

void Network::add_peer(shared_ptr<Peer> p)
{
  peers[ p->get_address() ] = p;
  // NB: decide whether PeerMap should have ID or IP as key
  // or whether to have a JoiningMap with IP and a ConnectedMap with ID
  Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                      asio::placeholders::error, p.get());
  p->start_listening(listen_handler);
}

void Network::add(string peer_name)
{
  tcp::resolver::query query(peer_name, itos(RAC_PORT));
  tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

  shared_ptr<tcp::socket> socket(new tcp::socket(*io_service));
  asio::connect(*socket, endpoint_iterator);

  shared_ptr<Peer> new_peer(new Peer(socket));
  add_peer(new_peer);
}

void Network::send_all(string message)
{
  // check_peers();
  DEBUG("Sending \"" << message << "\" to " << peers.size() << " peers.");

  PeerMap::iterator it;
  for (it=peers.begin(); it!=peers.end(); it++) {

    it->second->send(message);
  }
}

void Network::handle_incoming_message(const system::error_code& error,
                                      Peer* emitter)
{
  if (error == asio::error::eof) {
    DEBUG("Peer @" << emitter->get_address() << " disconnected.");
    peers.erase( emitter->get_id() );
  }
  else if (error) {
    DEBUG("Network::handle_incoming_message: " << error.message());
  }
  else {
    try {
      Message* message = parse_message(emitter->get_last_message());
      DEBUG("yay");
      message->display();
    }
    catch (MessageParseException& e) {
      DEBUG("Network::handle_incoming_message: " << e.what());
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
