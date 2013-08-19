#ifndef PEER_H
#define PEER_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "Config.hpp"
#include "Message.hpp"
#include "ReadyMessage.hpp"
#include "Network.hpp"

using namespace boost;
using boost::asio::ip::tcp;

class Peer
{
public:
  typedef PeerState State;
  typedef function<void (const system::error_code&)> Handler;
  

  Peer(shared_ptr<tcp::socket> _socket, bool _local = false);

  void set_state(State new_state);

  void init(string _id, string _pub_key);

  void start_listening(Handler _listen_handler);

  void listen();

  void receive();
  
  tcp::socket& get_socket() {
    return *socket;
  }

  const string get_address() {
    return (local ? "localhost" : socket->remote_endpoint().address().to_string());
  }

  const string get_id() {
    return id;
  }

  const string get_key() {
    return pub_id_key;
  }

  const State get_state() {
    return state;
  }

  const string get_last_message() {
    string ret(last_message);
    return ret;
  }

  void send(string message);
  // void send(shared_ptr<Message> message);
  void get_ready(const system::error_code& error, shared_ptr<ReadyMessage> message);
  
  void finish_write();

private:
  string id;
  string pub_id_key;
  shared_ptr<tcp::socket> socket;
  char last_message[MESSAGE_SIZE];
  State state;
  Handler listen_handler;
  bool local;
  
};


#endif
