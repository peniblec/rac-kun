#ifndef READY_NOTIF_MESSAGE_H
#define READY_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


struct ReadyNotifMessage : Message // a signal sent by a joining node to its
                                   // direct predecessors and successors, to
                                   // indicate that it's ready to be used as
                                   // their predecessor/successor
{
  ReadyNotifMessage();
  
  string serialize();
  void display();
};


#endif
