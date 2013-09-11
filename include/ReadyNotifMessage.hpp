#ifndef READY_NOTIF_MESSAGE_H
#define READY_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


struct ReadyNotifMessage : Message
{
  ReadyNotifMessage();
  
  string serialize();
  void display();
};


#endif
