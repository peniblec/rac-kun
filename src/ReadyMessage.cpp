#include <iostream>

#include "ReadyMessage.hpp"

ReadyMessage::ReadyMessage()
  : Message(MESSAGE_TYPE_READY)
{
}

string ReadyMessage::serialize()
{
  string s = Message::serialize();
  return s;
}

void ReadyMessage::display()
{
  Message::display();
}
