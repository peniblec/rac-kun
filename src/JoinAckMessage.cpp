#include <iostream>

#include "JoinAckMessage.hpp"

JoinAckMessage::JoinAckMessage(string _id, string _key)
  : Message(MESSAGE_TYPE_JOIN_ACK), id(_id), pub_k(_key)
{
}

string JoinAckMessage::serialize()
{
  string s = Message::serialize();
  s.append(id);
  s.append(pub_k);
  return s;
}

void JoinAckMessage::display()
{
  cout << "Peer " << id << " with key " << pub_k << " has added us to its view."  << endl;
}
