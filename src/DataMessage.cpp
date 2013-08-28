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
  cout << "Data from this message: " << data << endl;
}
