#ifndef READY_MESSAGE_H
#define READY_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


struct ReadyMessage : Message
{
  ReadyMessage();
  
  string serialize();
  void display();

};


#endif
