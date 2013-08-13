#ifndef READY_NOTIF_MESSAGE_H
#define READY_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


class ReadyNotifMessage : public Message
{
public:
  ReadyNotifMessage();
  
  string serialize();
  void display();

private:
};


#endif
