#include <boost/bind.hpp>

#include "Peer.hpp"
#include "Network.hpp"
#include "Utils.hpp"


Peer::Peer(shared_ptr<tcp::socket> _socket, bool _local)
  : socket(_socket), state(PEER_STATE_NEW), local(_local)
{
}

void Peer::set_state(Peer::State new_state)
{
  state = new_state;
  DEBUG("Peer @" << get_address() << " now has state " << PeerStateNames[state]
        << " (" << state << ").");
}

void Peer::init(string _id, string _pub_key) {
  id = _id;
  pub_id_key = _pub_key;
}

void Peer::start_listening(Peer::Handler _listen_handler)
{
  listen_handler = _listen_handler;
  listen();
}

void Peer::listen()
{
  // clear buffer
  memset(last_message, 0, MESSAGE_SIZE);
    
  socket->async_read_some(asio::buffer(last_message),
                          bind(listen_handler,
                               asio::placeholders::error,
                               asio::placeholders::bytes_transferred));
}

void Peer::send(string message)
{
  // TODO: if message.size() > MESSAGE_SIZE, store the rest somewhere and send
  //       it later

  socket->async_write_some(asio::buffer(message),
                           bind(&Peer::finish_write, this,
                                asio::placeholders::error));
}

void Peer::finish_write(const system::error_code& e)
{
  if (e)
    DEBUG("Peer::finish_write: " << e.message());
}

