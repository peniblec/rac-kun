#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>

#include "Config.hpp"
#include "LocalPeer.hpp"
#include "Peer.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Network
{
public:

  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  LocalPeer& p);
  
  void add_peer(shared_ptr<Peer> p);
  
  vector<shared_ptr<Peer> > get_peers() {
    return peers;
  }

  void add(string peer_name);
  
  void send_all(string message);

  void check_peers();

private:
  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  vector<shared_ptr<Peer> > peers;
  LocalPeer local_peer;
};


#endif
