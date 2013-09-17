#include "BCastMessage.hpp"

BCastMessage::BCastMessage(Message::Type _type)
  : Message(_type)
{
}

bool BCastMessage::is_broadcast()
{
  return true;
}
