#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <map>

#include "Config.hpp"
#include "LocalPeer.hpp"
#include "Peer.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Network
{
public:
  typedef map<string, shared_ptr<Peer> > PeerMap;


  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  LocalPeer& p);
  
  void add_peer(shared_ptr<Peer> p);
  
  const PeerMap get_peers() {
    return peers;
  }

  void add(string entry_point);
  
  void send_all(string message);

  void handle_incoming_message(const system::error_code& error,
                               Peer* emitter);

  // void check_peers();

private:
  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  PeerMap peers;
  // map<shared_ptr<Peer> > connected_peers;
  // map<shared_ptr<Peer> > joining_peers;
  
  LocalPeer& local_peer;
};


#endif
