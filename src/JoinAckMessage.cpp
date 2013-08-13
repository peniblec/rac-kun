#include <iostream>

#include "JoinAckMessage.hpp"

JoinAckMessage::JoinAckMessage(string _id, string _key)
  : Message(MESSAGE_TYPE_JOIN_ACK), id(_id), pub_k(_key)
{
}

string JoinAckMessage::serialize()
{
  string s;
  s.push_back(type);
  s.append(id);
  s.append(pub_k);
  return s;
}

void JoinAckMessage::display()
{
  cout << "Join acknowledgement from peer " << id << " with key " << pub_k << endl;
}
