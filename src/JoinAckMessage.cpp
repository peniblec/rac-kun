#include <iostream>

#include "JoinAckMessage.hpp"

JoinAckMessage::JoinAckMessage(string _group_id, string _peer_id, string _key)
  : Message(MESSAGE_TYPE_JOIN_ACK),
    group_id(_group_id), peer_id(_peer_id), pub_k(_key)
{
}

string JoinAckMessage::serialize()
{
  string s = Message::serialize();
  s.append(peer_id);
  s.append(pub_k);
  s.append(group_id);
  return s;
}

void JoinAckMessage::display()
{
  cout << "Peer " << peer_id << " with key " << pub_k << " has added us to its view."  << endl;
}
