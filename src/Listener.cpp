#include <boost/bind.hpp>

#include "Listener.hpp"
#include "Utils.hpp"


Listener::Listener(shared_ptr<asio::io_service> io_service,
                   shared_ptr<Network> _network)
  : acceptor(*io_service, tcp::endpoint(tcp::v4(), settings.LISTEN_PORT)),
    network(_network)
{
  DEBUG("Now listening on port " << acceptor.local_endpoint().port() << ".");

  // if LISTEN_PORT was 0, Boost.Asio chose a random port
  settings.LISTEN_PORT = acceptor.local_endpoint().port();

  start_accept();
}

void Listener::start_accept()
{
  shared_ptr<tcp::socket> socket(new tcp::socket(acceptor.get_io_service()));

  shared_ptr<Peer> new_peer(new Peer(socket));

  acceptor.async_accept(*socket, bind(&Listener::handle_accept, this, new_peer,
                                      asio::placeholders::error) );
}

void Listener::handle_accept(shared_ptr<Peer> new_peer,
                             const system::error_code& error)
{
  if (!error) {
    network->add_new_peer(new_peer);

    DEBUG("Listener::handle_accept: Now in touch with peer @"
          << new_peer->get_address() << ":" << new_peer->get_port() << ".");
  }
  else {
    DEBUG("Listener::handle_accept: " << error.message());
  }
  start_accept();
}
