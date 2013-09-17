#include <iostream>

#include "DataMessage.hpp"

DataMessage::DataMessage(string _data)
  : BCastMessage(MESSAGE_TYPE_DATA), data(_data)
{
}

string DataMessage::serialize()
{
  string s = Message::serialize();
  s.append(BCAST_MARKER);
  s.append(data);
  return s;
}

void DataMessage::display()
{
  Message::display();
  cout << "\tData: " << data << endl;
}
