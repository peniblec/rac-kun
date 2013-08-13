#include <iostream>

#include "ReadyNotifMessage.hpp"

ReadyNotifMessage::ReadyNotifMessage()
  : Message(MESSAGE_TYPE_READY_NOTIF)
{
}

string ReadyNotifMessage::serialize()
{
  string s;
  s.push_back(type);
  return s;
}

void ReadyNotifMessage::display()
{
  cout << "Emitter is now ready to be used as relay." << endl;
}
