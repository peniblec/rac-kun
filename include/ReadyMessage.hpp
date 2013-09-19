#ifndef READY_MESSAGE_H
#define READY_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


struct ReadyMessage : Message // a signal sent by the entry point to the joining
                              // peer, to indicate that all group members should
                              // have made themselves known by now
{
  ReadyMessage();
  
  string serialize();
  void display();

};


#endif
