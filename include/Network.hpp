#ifndef NETWORK_H
#define NETWORK_H

#include <boost/asio.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <map>

#include "Config.hpp"
#include "Group.hpp"
#include "Message.hpp"
#include "BCastMessage.hpp"
#include "Peer.hpp"
#include "Ring.hpp"
#include "Utils.hpp"

#define LOG_INDEX_TIME 0
#define LOG_INDEX_HASH 1

using namespace boost;
using namespace boost::multi_index;
using boost::asio::ip::tcp;

class Network
{
private:
  typedef map<string, pair<shared_ptr<Peer>, unsigned short> > JoinMap;
  // associates IP with peer/listening port
  typedef map<string, shared_ptr<Group> > GroupMap;

  struct MessageLog {
    string message;
    map<string, int> control;
    // associates a peer ID to the number of times
    // we received this message from this peer

    bool operator<(const MessageLog ml)const {
      string my_stamp(message, 1, MSG_STAMP_LENGTH);
      string its_stamp(ml.message, 1, MSG_STAMP_LENGTH);

      return my_stamp<its_stamp;
      // return ( memcmp( (message.c_str())[1],
      //                  (ml.message.c_str())[1],
      //                  MSG_STAMP_LENGTH ) < 0 );
      
    }
  };

  // struct used to modify the content of the multi index container;
  // adds 1 to the number of times we received a message from _peer
  struct ack_message {
    ack_message(shared_ptr<Peer> _peer)
      : peer(_peer) {}

    void operator() (MessageLog& ml) {
      (ml.control[ peer->get_id() ])++;
    }
  private:
    shared_ptr<Peer> peer;
  };

  // multi index container keeping the logs;
  // may be accessed with two iterators:
  // - sequential (order of reception)
  // - associative (using the message stamp as key)
  typedef multi_index_container<
    MessageLog,
    indexed_by<
      sequenced<>,
      ordered_unique<identity<MessageLog> >
      >
    > History;
  typedef History::nth_index<LOG_INDEX_TIME>::type LogIndexTime;
  typedef History::nth_index<LOG_INDEX_HASH>::type LogIndexHash;


public:
  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  shared_ptr<Peer> _local_peerp);
  
  // Join-related methods

  /* join:
     - called by UI or during automatic configuration
     - used to join an existing system

     - entry_ip, entry_port: endpoint info about the entry point
         (entry_ip may be ip or hostname)
  */
  void join(string entry_ip, string entry_port);

  /* answer_join_request:
     - called when receiving a join request and the system is not
       waiting for another node to complete the procedure
     - broadcast a Join Notification to the appropriate group
     - setup timer before sending Ready message

     - peer: the new peer joining the system
     - port: the peer's listening port
   */
  void answer_join_request(shared_ptr<Peer> peer, unsigned short port);

  /* acknowledge_join:
     - called when receiving a Join Notification
     - add peer to group rings, send him a join acknowledgement
     - if not a direct pred/succ, set up the timer before considering
       them connected

     - peer: the new peer joining the system
   */
  void acknowledge_join(shared_ptr<Peer> peer, shared_ptr<Group> group);

  /* complete_join:
     - handler called after join timer expired
     - set peer state to CONNECTED, check whether there are pending peers
       waiting to join

     - error: some asynchronous error raised by boost
     - peer: the peer who completed the join procedure
   */
  void complete_join(const system::error_code& error, shared_ptr<Peer> peer);



  // Peer-related methods

  /* connect_peer:
     - called when trying to join/when adding a joining node to our view
     - use a hostname/ip and port to create a Peer and its associated socket

     - {ip: ip or hostname; port}: info about the node to reach
     - returns the new peer, unidentified yet
   */
  shared_ptr<Peer> connect_peer(string ip, string port);
  
  /* add_new_peer:
     - called by Listener, or when contacting entry point
     - add an unidentified peer to new_peers map, start listening to them

     - p: the peer to add to new_peers
   */
  void add_new_peer(shared_ptr<Peer> p);

  /* check_for_new_peers:
     - called whenever a new peer has completed the join procedure
     - check whether any peer in new_peers is identified (ie has sent a
       join request), in which case, start join procedure again
   */
  void check_for_new_peers();

  /* handle_disconnect:
     - called when eof is read by asio
     - remove peer from maps and rings

     - p: the peer to remove
   */
  void handle_disconnect(shared_ptr<Peer> p);



  // Message-sending-related methods

  /* broadcast:
     - send a message to all successors in the group/channel rings
     - log the message

     - group: if local, message will be broadcast in this group's rings,
         otherwise, in the channel rings
     - message: the message to send
     - add_stamp: generate a new ID for this message
         default is false, since when simply passing a message down the rings,
         the ID should not be tampered with
   */
  void broadcast(shared_ptr<Group> group, BCastMessage* message, bool add_stamp=false);

  /* broadcast_data:
     - called by UI
     - create a data message and broadcast it to local group

     - content: what the user wants to send
   */
  void broadcast_data(string content);

  /* send_all:
     - send cleartext to all members simultaneously (not using the rings)

     - message: what the user wants to send
   */
  void send_all(string message);

  /* send:
     - serialize message, send it to one peer in particular, log the message

     - message: some message conforming to the types defined in this protocol
     - peer: the peer to send this message to
   */
  void send(Message* message, shared_ptr<Peer> peer);

  /* send_ready:
     - handler called after the ready timer expired
     - when enough time has passed, assume every member has added the new member
       to their view, and send the guy a ready message
     - erase the ready timer
       
     - error: some asynchronous error raised by boost
     - peer: the peer going through the join procedure
   */
  void send_ready(const system::error_code& error, shared_ptr<Peer> peer);



  // Other message-related methods

  /* handle_incoming_message:
     - handler for asynchronous readings from all peers
     - determine the kind of message, log it, forward it if it's broadcast
     - depending on the type of message, process it

     - error: some asynchronous error raised by boost
     - bytes_transferred: used to build a string from char* and keeping null bytes
     - emitter: the peer who sent this message
   */
  void handle_incoming_message(const system::error_code& error,
                               size_t bytes_transferred,
                               shared_ptr<Peer> emitter);

  /* log_message:
     - called whenever a valid message is processed, or sent
     - if the message is already logged, acknowledge that we've received it from
       this emitter
     - otherwise, log it, and keep track of whether all our predecessors have
       forwarded it

     - message: the message to log
     - emitter: whom we received the message from
   */
  void log_message(Message* message, shared_ptr<Peer> emitter);



  // Other methods
  
  /* print_rings:
     - called by UI
     - for each ring, display members sorted according to their position
     - display our predecessors/successors
   */
  void print_rings();

  /* print_logs
     - called by UI
     - print all messages we've received/sent so far, along with who sent it to us
       and how many times
   */
  void print_logs();


private:

  /* find_log
     - find message in log history

     - message: the message to find
   */
  LogIndexHash::iterator find_log(Message* message);

  shared_ptr<asio::io_service> io_service;
  shared_ptr<tcp::resolver> resolver;

  PeerMap peers; // all members except local_peer
  shared_ptr<Peer> local_peer;

  GroupMap groups; // all groups, sorted by ID, including local
  shared_ptr<Group> local_group;

  map<string, string> channel_markers; // associates a channel ID to a group
  // maps local_group id to local_group id

  JoinMap new_peers; // map storing information about pending Join Requests
  bool join_token; // false when a JOIN procedure is ongoing

  History logs; // sorted with Message.stamp
  LogIndexHash& h_logs; // associative access (retrieve message by stamp)
  LogIndexTime& t_logs; // sequential access (list messages in order of reception)

  map<string, shared_ptr<asio::deadline_timer> > ready_timers;
  // associates a peer (ID) with a timer before sending a READY message
  map<string, shared_ptr<asio::deadline_timer> > join_timers;
  // associates a peer (ID) with a timer before using the peer as relay  

};


#endif
