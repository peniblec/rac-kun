#include <boost/bind.hpp>

#include "Peer.hpp"
#include "Utils.hpp"


Peer::Peer(shared_ptr<tcp::socket> _socket)
  : socket(_socket), state(PEER_STATE_CONNECTING)
{
}

void Peer::set_state(Peer::State new_state)
{
  state = new_state;
  DEBUG("Peer @" << get_address() << " now has state " << PeerStateNames[state] << ".");
}

void Peer::start_listening()
{
  set_state(PEER_STATE_ALIVE);
  listen();
}

void Peer::listen()
{
  memset(last_message, 0, MESSAGE_SIZE);
    
  socket->async_read_some(asio::buffer(last_message),
                          bind(&Peer::handle_incoming_message, this,
                               asio::placeholders::error));
}

void Peer::handle_incoming_message(const system::error_code& error)
{
  if (error == asio::error::eof) {
    DEBUG("Peer @" << get_address() << " disconnected.");
    set_state(PEER_STATE_DEAD);
  }
  else {
    cout << "- " << get_address() << ": " << get_last_message() << endl;
    listen();
  }
}

void Peer::send(string message)
{
  asio::async_write(*socket, asio::buffer(message),
                    bind(&Peer::finish_write, this));
}

void Peer::finish_write()
{
  
}

