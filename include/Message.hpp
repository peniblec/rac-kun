#ifndef MESSAGE_H
#define MESSAGE_H

#include "Config.hpp"

class Message
{
public:
  typedef MessageType Type;

  Message();
  string serialize();

protected:
  Type type;
};


#endif
