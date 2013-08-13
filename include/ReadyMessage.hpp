#ifndef READY_MESSAGE_H
#define READY_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


class ReadyMessage : public Message
{
public:
  ReadyMessage();
  
  string serialize();
  void display();

private:
};


#endif
