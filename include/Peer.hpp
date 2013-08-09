#ifndef PEER_H
#define PEER_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "Config.hpp"
#include "Network.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Peer
{
public:
  typedef PeerState State;
  typedef function<void (const system::error_code&)> Handler;
  

  Peer(shared_ptr<tcp::socket> _socket);

  void set_state(State new_state);

  void start_listening(Handler _listen_handler);

  void listen();

  void receive();
  
  tcp::socket& get_socket() {
    return *socket;
  }

  const string get_address() {
    return socket->remote_endpoint().address().to_string();
  }

  const string get_id() {
    return id;
  }

  const string get_last_message() {
    string ret(last_message);
    return ret;
  }

  void send(string message);

  void finish_write();

  bool is_alive() {
    return (state!=PEER_STATE_DEAD);
  }


private:
  string id;
  shared_ptr<tcp::socket> socket;
  char last_message[MESSAGE_SIZE];
  State state;
  Handler listen_handler;
  
};


#endif
