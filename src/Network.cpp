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
  peers.push_back(p);
  p->start_listening();
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
  check_peers();
  DEBUG("Sending \"" << message << "\" to " << peers.size() << " peers.");

  for (uint p=0 ; p<peers.size() ; p++) {

    peers[p]->send(message);
  }
}

void Network::check_peers()
{
  for (uint p=0 ; p<peers.size() ; p++) {

    while (peers.size() > p  && ! peers[p]->is_alive() ) {
      peers.erase( peers.begin()+p );
      DEBUG("Removed peer, " << peers.size() << " remaining.");
    }
  }
}
