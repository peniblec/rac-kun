#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <map>

#include "Config.hpp"
#include "LocalPeer.hpp"
#include "Peer.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Peer;

class Network
{
public:
  typedef map<string, shared_ptr<Peer> > PeerMap;


  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  LocalPeer& p);
  
  void add_new_peer(shared_ptr<Peer> p, PeerMap& m);
  void add_new_peer(shared_ptr<Peer> p) {
    add_new_peer(p, new_peers);
  }

  void join(string entry_point);
  shared_ptr<Peer> connect_peer(string peer_name);
  
  const PeerMap get_peers() {
    return peers;
  }

  void send_all(string message);
  void send_ready(const system::error_code& error, shared_ptr<Peer> peer);

  void handle_incoming_message(const system::error_code& error,
                               shared_ptr<Peer> emitter);

private:
  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  PeerMap peers; // sorted with ID
  PeerMap new_peers; // sorted with IP
  PeerMap joining_peers; // sorted with ID
  
  LocalPeer& local_peer;

  map<string, shared_ptr<asio::deadline_timer> > ready_timers;
};


#endif
