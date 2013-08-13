#include <iostream>

#include "JoinNotifMessage.hpp"

JoinNotifMessage::JoinNotifMessage(string _id, string _key, string _ip)
  : Message(MESSAGE_TYPE_JOIN_NOTIF), id(_id), pub_k(_key), ip(_ip)
{
}

string JoinNotifMessage::serialize()
{
  string s;
  s.push_back(type);
  s.append(id);
  s.append(pub_k);
  s.append(ip);
  return s;
}

void JoinNotifMessage::display()
{
  cout << "Peer with id " << id << " (" << ip << ") and key " << pub_k << " wants to join." << endl;
}
