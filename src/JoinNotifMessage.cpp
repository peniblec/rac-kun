#include <iostream>

#include "JoinNotifMessage.hpp"
#include "Utils.hpp"

JoinNotifMessage::JoinNotifMessage(string _id, string _key,
                                   string _ip, unsigned short _port)
  : Message(MESSAGE_TYPE_JOIN_NOTIF), id(_id), pub_k(_key), ip(_ip), port(_port)
{
}

string JoinNotifMessage::serialize()
{
  string s = Message::serialize();
  s.append(id);
  s.append(pub_k);
  s.append(ip);
  s.push_back(':');
  s.append( itos(port) );
  
  return s;
}

void JoinNotifMessage::display()
{
  cout << "Peer with id " << id << " (" << ip << ":" << port
       << ") and key " << pub_k << " wants to join." << endl;
}
