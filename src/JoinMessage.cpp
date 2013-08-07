#include "JoinMessage.hpp"

JoinMessage::JoinMessage()
  : type(MESSAGE_TYPE_JOIN)
{
}

JoinMessage::JoinMessage(string raw)
  : type(MESSAGE_TYPE_JOIN)
{
  id = string(raw, JOIN_ID_MSG_OFFSET,
              JOIN_MSG_ID_LENGTH);
  pub_k = string(raw, JOIN_MSG_KEY_OFFSET,
                 JOIN_MSG_KEY_LENGTH);
}

JoinMessage::JoinMessage(string _id, string _key)
  : type(MESSAGE_TYPE_JOIN), id(_id), pub_k(_key)
{
}

string JoinMessage::serialize()
{
  string s;
  s.append(type);
  s.append(id);
  s.append(pub_k);
}
