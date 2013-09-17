#ifndef BCAST_MESSAGE_H
#define BCAST_MESSAGE_H

#include "Message.hpp"


struct BCastMessage : Message
{
  bool is_broadcast();

  string BCAST_MARKER;

protected:
  BCastMessage(Message::Type _type);
};

#endif
