#include <iostream>

#include "JoinMessage.hpp"

// JoinMessage::JoinMessage()
//   : type(MESSAGE_TYPE_JOIN)
// {
// }

// JoinMessage::JoinMessage(string raw)
//   : type(MESSAGE_TYPE_JOIN)
// {
//   id = string(raw, JOIN_ID_MSG_OFFSET,
//               JOIN_MSG_ID_LENGTH);
//   pub_k = string(raw, JOIN_MSG_KEY_OFFSET,
//                  JOIN_MSG_KEY_LENGTH);
// }

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
  cout << "Join message from peer " << id << " with key " << pub_k << endl;
}
