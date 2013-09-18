#ifndef LISTENER_H
#define LISTENER_H

#include <boost/asio.hpp>
#include <list>

#include "Config.hpp"
#include "Network.hpp"
#include "Peer.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Listener // An acceptor listening on a predefined port, establishes TCP
               // connections, and sends the resulting new peer to Network
{

public:
  Listener(shared_ptr<asio::io_service> io_service,
           shared_ptr<Network> _network);

private:
  /* start_accept:
     - creates a new socket, and makes an asynchronous accept (setting
       handle_accept as the handler)
  */
  void start_accept();

  /* handle_accept:
     - handler for asynchronous accept
     - sends the new peer to Network
     - calls start_accept again
  */
  void handle_accept(shared_ptr<Peer> new_peer,
                     const system::error_code& error);


  tcp::acceptor acceptor; // an object accepting connections on a predefined
                          // port
  shared_ptr<Network> network; // the program's view of the network ; the entity
                               // managing new peers
  shared_ptr<Peer> pending_peer; // the "blank peer" whose socket is waiting to
                                 // be used
};

#endif
