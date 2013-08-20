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
  memset(last_message, 0, MESSAGE_SIZE);
    
  socket->async_read_some(asio::buffer(last_message),
                          bind(listen_handler, asio::placeholders::error));
}

void Peer::receive()
{
    cout << "- " << get_address() << ": " << get_last_message() << endl;
}

void Peer::send(string message)
{
  // TODO: limit buffer size to MESSAGE_SIZE, loop until whole is sent
  socket->async_write_some(asio::buffer(message),
                           bind(&Peer::finish_write, this));
}

// void Peer::send(shared_ptr<Message> message)
// {
//   socket->async_write_some(asio::buffer( message->serialize() ),
//                            bind(&Peer::finish_write, this));
// }

void Peer::finish_write()
{
  
}

