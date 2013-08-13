#include <iostream>

#include "JoinMessage.hpp"

JoinMessage::JoinMessage(string _id, string _key)
  : Message(MESSAGE_TYPE_JOIN), id(_id), pub_k(_key)
{
}

string JoinMessage::serialize()
{
  string s;
  s.push_back(type);
  s.append(id);
  s.append(pub_k);
  return s;
}

void JoinMessage::display()
{
  cout << "Peer with id " << id << " and key " << pub_k << " wants to join." << endl;
}
