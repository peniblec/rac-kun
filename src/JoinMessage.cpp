#include <iostream>

#include "JoinMessage.hpp"
#include "Utils.hpp"

JoinMessage::JoinMessage(string _id, string _key, unsigned short _port)
  : Message(MESSAGE_TYPE_JOIN), id(_id), pub_k(_key), port(_port)
{
}

string JoinMessage::serialize()
{
  string s = Message::serialize();
  s.append(id);
  s.append(pub_k);
  s.append( itos(port) );
  return s;
}

void JoinMessage::display()
{
  cout << "Peer with id " << id << " and key " << pub_k
       << " wants to join; listening port: " << port << "." << endl;
}
