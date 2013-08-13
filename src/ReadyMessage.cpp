#include <iostream>

#include "ReadyMessage.hpp"

ReadyMessage::ReadyMessage()
  : Message(MESSAGE_TYPE_READY)
{
}

string ReadyMessage::serialize()
{
  string s;
  s.push_back(type);
  return s;
}

void ReadyMessage::display()
{
  cout << "Ready message." << endl;
}
