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

class Network // the program's view of the various entities we are connected to
              // (groups, channels, peers, ...), where messages are processed
              // and archived
{
private:
  typedef map<string, pair<shared_ptr<Peer>, unsigned short> > JoinMap;
  // associates an IP address with the pending peer and its listening port
  // TODO: would probably need to use pair<IP, I/O port> as key to be more
  //       correct 

  typedef map<string, shared_ptr<Group> > GroupMap; // associates a string (ID)
                                                    // with a group

  struct MessageLog // stores a message, and keeps track of the number of times
                    // we've received it from our predecessors (and/or possibly
                    // other peers)
  {
    string message;
    map<string, int> control; // associates a peer ID with the number of times
                              // this peer sent us this message
    /* operator<:
       - the comparison method for stored messages
       - uses messages stamps (see Message class) for comparison
     */
    bool operator<(const MessageLog ml)const
    { 
      string my_stamp(message, 1, MSG_STAMP_LENGTH);
      string its_stamp(ml.message, 1, MSG_STAMP_LENGTH);
      return my_stamp<its_stamp;
      // return ( memcmp( (message.c_str())[1],
      //                  (ml.message.c_str())[1],
      //                  MSG_STAMP_LENGTH ) < 0 );
      
    }
  };

  struct ack_message // functor used to acknowledge the reception of a message
                     // from a peer
  {
    ack_message(shared_ptr<Peer> _peer)
      : peer(_peer) {}

    void operator() (MessageLog& ml) {
      (ml.control[ peer->get_id() ])++;
    }
  private:
    shared_ptr<Peer> peer;
  };


  typedef multi_index_container <
    MessageLog,
    indexed_by<
      sequenced<>,
      ordered_unique<identity<MessageLog> >
      >
    > History; // multi index container keeping the logs, which may be accessed
               // with two iterators:
               // - sequential (order of reception)
               // - associative (using the message stamp as key)
  typedef History::nth_index<LOG_INDEX_TIME>::type LogIndexTime;
  typedef History::nth_index<LOG_INDEX_HASH>::type LogIndexHash;


public:
  Network(shared_ptr<asio::io_service> _ios,
          shared_ptr<tcp::resolver> _resolver,
	  shared_ptr<Peer> _local_peep);
  

  // Join-related methods

  /* join:
     - called by UI or during automatic configuration
     - used to join an existing system

     - entry_ip, entry_port: endpoint info about the entry point
       (entry_ip may be ip or hostname)
  */
  void join(string entry_ip, string entry_port);

  /* answer_join_request:
     - called when receiving a join request and the system is not waiting for
       another node to complete the procedure
     - broadcast a Join Notification to the appropriate group
     - setup timer before sending READY signal

     - peer: the new peer joining the system
     - port: the peer's listening port, to communicate to members of the group
   */
  void answer_join_request(shared_ptr<Peer> peer, unsigned short port);

  /* acknowledge_join:
     - called when handling a Join Notification
     - add peer to the group, update group and channel rings accordingly
     - send him a join acknowledgement
     - if not a direct pred/succ, set a timer before considering him connected

     - peer: the new peer joining the system
     - group: the group this peer should join (assumed to be correct)
   */
  void acknowledge_join(shared_ptr<Peer> peer, shared_ptr<Group> group);

  /* complete_join:
     - handler called after join timer expired
     - set peer state to CONNECTED, calls check_for_new_peers

     - error: some asynchronous error raised by boost
     - peer: the peer who completed the join procedure
   */
  void complete_join(const system::error_code& error, shared_ptr<Peer> peer);

  /* check_for_new_peers:
     - called whenever a new peer has completed the join procedure
     - check whether any peer in new_peers is identified (ie has sent a JOIN
       request), in which case, start JOIN procedure again
   */
  void check_for_new_peers();



  // Peer-related methods

  /* connect_peer:
     - called when trying to join/when adding a joining node to our view
     - use a hostname/ip and port to create a Peer and its associated socket

     - ip, port: info about the node to reach ("ip" may be the IP address, or a
       hostname) 
     - returns the new peer, unidentified yet (ie state = NEW, ID and key
       unknown)
   */
  shared_ptr<Peer> connect_peer(string ip, string port);
  
  /* add_new_peer:
     - called by Listener, or when contacting entry point
     - add an unidentified peer to new_peers map, start listening to them

     - p: the peer to add to new_peers
   */
  void add_new_peer(shared_ptr<Peer> p);

  /* handle_disconnect:
     - called when EOF is read by asio
     - remove peer from maps and rings, update neighbours accordingly
     - if we are alone, set ourselves as NEW again

     - p: the peer to remove
   */
  void handle_disconnect(shared_ptr<Peer> p);



  // Message-sending-related methods

  /* broadcast:
     - send a message to all successors in the group/channel
     - log the message

     - group: if local, message will be sent to our successors in this group's
       rings; otherwise, in the channel rings
     - message: the message to send; presumed complete (stamp excluded): in
       particular, the BCAST_MARKER should have been set
     - add_stamp: generate a new ID for this message (default: false, since the
       ID should not be tampered with when we are simply passing the message
       down the rings)
   */
  void broadcast(shared_ptr<Group> group, BCastMessage* message,
                 bool add_stamp=false);

  /* broadcast_data:
     - called by UI
     - create a data message and broadcast it to local group

     - content: what the user wants to send
   */
  void broadcast_data(string content);

  /* send_all:
     - called by UI
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
     - when enough time has passed, assume every member has added the new node
       to their view, and send the guy a READY signal
       
     - error: some asynchronous error raised by boost
     - peer: the peer going through the JOIN procedure
   */
  void send_ready(const system::error_code& error, shared_ptr<Peer> peer);



  // Other message-related methods

  /* handle_incoming_message:
     - handler for asynchronous readings from all peers
     - determine the kind of message, log it, forward it if it's broadcast
     - depending on the type of message, process it

     - error: some asynchronous error raised by boost
     - bytes_transferred: the number of bytes to read (used to prevent
       std::string to stop at null bytes)
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



  // Group-related methods
  
  /* create_group:
     - called when creating a group, or receiving Join Acknowledgements
     - create a new group with the specified ID, adds it to our list of groups

     - id: the ID the group should have
     - returns the newly created group
     /!\ does not check whether insertion into the map was successful (will fail
         if a group with this ID already exists)
     /!\ does not create the channel marker associated with this group
   */
  shared_ptr<Group> create_group(string id);

  /* make_channel_marker:
     - called after creating a new group
     - combines two group IDs to generate a marker which will be used in
       broadcast messages to indicate in which group/channel they belong
   */
  string make_channel_marker(shared_ptr<Group> group1,
                             shared_ptr<Group> group2);


  // Other methods:
  
  /* print_rings:
     - called by UI
     - for each group, display all rings (see Ring), which are either the local
       group rings (for group==local_group) or the associated channel rings
   */
  void print_rings();

  /* print_logs
     - called by UI
     - print all messages we've received/sent so far, along with who sent it to
       us and how many times
   */
  void print_logs();


private:

  /* find_log:
     - find message in log history

     - message: the message to find
     - returns an iterator to the message
   */
  LogIndexHash::iterator find_log(Message* message);

  shared_ptr<asio::io_service> io_service; // a Boost.Asio object which will let
                                           // us create sockets, timers...
  shared_ptr<tcp::resolver> resolver; // a probe which can find the endpoint
                                      // associated with a couple <host, port>

  PeerMap peers; // all members except local_peer
  shared_ptr<Peer> local_peer;

  GroupMap groups; // all groups, sorted by ID, including local
  shared_ptr<Group> local_group;

  map<string, string> channel_markers; // associates a channel ID to a group ID;
                                       // used to associate a broadcast marker
                                       // found on a message with the group this
                                       // message should be forwarded to

                                       // (the local group also has a "channel
                                       //  marker"; it is generated with a call
                                       //  to make_channel_marker(local group,
                                       //  local group)

  JoinMap new_peers; // map storing information about pending JOIN Requests
  bool join_token; // token grabbed whenever a peer is joining the system;
                   // released when the JOIN procedure is complete

  History logs; // sorted with Message.stamp
  LogIndexHash& h_logs; // associative access (retrieve message by stamp)
  LogIndexTime& t_logs; // sequential access (list by order of reception) 

  map<string, shared_ptr<asio::deadline_timer> > ready_timers;
  // associates a peer (ID) with a timer before sending a READY message
  map<string, shared_ptr<asio::deadline_timer> > join_timers;
  // associates a peer (ID) with a timer before using the peer as relay  
};


#endif
