#include <iostream>

#include "ReadyNotifMessage.hpp"

ReadyNotifMessage::ReadyNotifMessage()
  : Message(MESSAGE_TYPE_READY_NOTIF)
{
}

string ReadyNotifMessage::serialize()
{
  string s = Message::serialize();
  return s;
}

void ReadyNotifMessage::display()
{
  Message::display();
}
