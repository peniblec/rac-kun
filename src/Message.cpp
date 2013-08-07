#include "Message.hpp"

Message::Message()
  : type(MESSAGE_TYPE_NONE)
{
}

string Message::serialize()
{
  string s;
  s.append(type);
}
