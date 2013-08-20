#include <boost/bind.hpp>

#include "Listener.hpp"
#include "Utils.hpp"


Listener::Listener(shared_ptr<asio::io_service> io_service, shared_ptr<Network> _network)
  : acceptor(*io_service, tcp::endpoint(tcp::v4(), RAC_PORT)),
    network(_network)
{
  start_accept();
}

void Listener::start_accept()
{
  shared_ptr<tcp::socket> socket(new tcp::socket(acceptor.get_io_service()));

  Peer* new_peer(new Peer(socket));

  list<Peer*>::iterator new_peer_it;
  new_peer_it = pending_peers.insert(pending_peers.begin(), new_peer);

  acceptor.async_accept(new_peer->get_socket(),
                        bind(&Listener::handle_accept, this, new_peer_it,
                             asio::placeholders::error) );
}

void Listener::handle_accept(list<Peer*>::iterator new_peer_it,
                             const system::error_code& error)
{
  if (!error) {
    shared_ptr<Peer> new_peer(*new_peer_it);

    network->add_new_peer(new_peer);

    pending_peers.erase(new_peer_it);

    DEBUG("Listener::handle_accept: Now in touch with peer @" << new_peer->get_address() << ".");

    start_accept();
  }
  else {
    DEBUG("Listener::handle_accept: " << error.message());
  }
        
}
