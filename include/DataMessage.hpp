#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct DataMessage : Message
{
  DataMessage(string _data);
  
  string serialize();
  void display();

  const string data;
};


#endif
