#include <iostream>

#include "DataMessage.hpp"

DataMessage::DataMessage(string _data)
  : Message(MESSAGE_TYPE_DATA), data(_data)
{
}

string DataMessage::serialize()
{
  string s = Message::serialize();
  s.append(data);
  return s;
}

void DataMessage::display()
{
  Message::display();
  cout << "\tData: " << data << endl;
}
