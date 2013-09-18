#ifndef PEER_H
#define PEER_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "Config.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Peer // represents a member of the system, or at least, a node attempting
           // to join it
{
public:
  typedef PeerState State;

  typedef function<void (const system::error_code&, size_t)> Handler;
  // a handler for asynchronous reads

  Peer(shared_ptr<tcp::socket> _socket, bool _local = false);

  /* init:
     - fills basic information about the peer

     - _id: the unique peer ID
     - _pub_key: the public ID key
   */
  void init(string _id, string _pub_key);

  /* start_listening:
     - sets the listen handler for this peer, then calls listen()

     - _listen_handler: the handler for asynchronous reads coming from this peer
   */
  void start_listening(Handler _listen_handler);

  /* listen:
     - calls an asynchronous read on this peer's socket
   */
  void listen();

  /* get_last_message;
     - get read buffer content

     - len: the number of bytes to read from the buffer; if 0, stop at first
       null byte 
     - returns the string generated from the buffer
   */
  const string get_last_message(size_t len = 0) {
    return ( len ? string(last_message, len) : string(last_message) );
  }

  /* send:
     - start an asynchronous write on this peer's socket

     - message: the bytes to send
   */
  void send(string message);
  
  /* finish_write:
     - handler for completion of write event
     - if there were any error, report them

     - e: some error raised by Boost.Asio
   */
  void finish_write(const system::error_code& e);
  
  const string get_address() {
    return (local ? "localhost"
            : socket->remote_endpoint().address().to_string());
  }

  const unsigned short get_port() {
    return (local ? settings.LISTEN_PORT
            : socket->remote_endpoint().port());
  }

  const string get_id() {
    return id;
  }

  const string get_key() {
    return pub_id_key;
  }

  void set_state(State new_state);
  const State get_state() {
    return state;
  }
  
  bool is_known() {
    return (state != PEER_STATE_NEW);
  }

  bool is_local() {
    return local;
  }


private:
  string id; // a unique ID for this peer, computed as per RAC.III.C.Joining

  string pub_id_key; // dummy public id key, used as placeholder for messages

  shared_ptr<tcp::socket> socket; // the socket used to communicate with this
                                  // peer 

  char last_message[MESSAGE_SIZE]; // the buffer used to store the bytes this
                                   // peer sends to us

  State state; // the current state of this peer wrt the JOIN procedure; see
               // Config.hpp 

  Handler listen_handler; // the handler for asynchronous reads on this peer's
                          // socket

  bool local; // whether this peer is local or distant
  
};


#endif
