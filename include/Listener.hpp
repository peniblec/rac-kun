/*

  RAC-kun - junior version of RAC, a freerider-resilient, scalable, anonymous
  communication protocol conceived by researchers from CNRS and LIG.
  Copyright (C) 2013  Kévin Le Gouguec

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see [http://www.gnu.org/licenses/].

*/
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
