#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <map>
#include <set>

#include "Config.hpp"
#include "Peer.hpp"
#include "Ring.hpp"

#define LOG_INDEX_TIME 0
#define LOG_INDEX_HASH 1

using namespace boost;
using namespace boost::multi_index;
using boost::asio::ip::tcp;

class Network
{
public:
  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  shared_ptr<Peer> p);
  
  void add_new_peer(shared_ptr<Peer> p);

  void add_peer_to_rings(shared_ptr<Peer> p);

  void update_my_neighbours();

  void join(string entry_point);

  shared_ptr<Peer> connect_peer(string peer_name);
  
  void handle_join(shared_ptr<Peer> peer);

  void send_all(string message);

  void send_ready(const system::error_code& error, shared_ptr<Peer> peer);

  void handle_incoming_message(const system::error_code& error,
                               size_t bytes_transferred,
                               shared_ptr<Peer> emitter);

  void print_rings();

  void broadcast(string msg);

  void print_logs();

private:
  typedef map<string, shared_ptr<Peer> > PeerMap;
  typedef set<shared_ptr<Peer> > PeerSet;  

  struct MessageLog {
    string message;
    // map< string, pair<shared_ptr<Peer>, int> > emitters;
    shared_ptr<Peer> emitter;

    bool operator<(const MessageLog ml)const {
      string my_stamp(message, MSG_STAMP_OFFSET, MSG_STAMP_LENGTH);
      string its_stamp(ml.message, MSG_STAMP_OFFSET, MSG_STAMP_LENGTH);
      return my_stamp<its_stamp;
    }
  };
  typedef multi_index_container<
    MessageLog,
    indexed_by<
      sequenced<>,
      ordered_unique<identity<MessageLog> >
      >
    > History;
  typedef History::nth_index<LOG_INDEX_TIME>::type::iterator LogTimeIterator;
  typedef History::nth_index<LOG_INDEX_HASH>::type::iterator LogHashIterator;

  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  PeerMap peers; // sorted with ID
  PeerMap new_peers; // sorted with IP
  
  shared_ptr<Peer> local_peer;

  PeerSet predecessors;
  PeerSet successors;

  History logs; // sorted with Message.stamp

  map<string, shared_ptr<asio::deadline_timer> > ready_timers;

  Ring rings[RINGS_NB];
  
};


#endif
