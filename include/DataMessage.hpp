#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include "Config.hpp"

#include "BCastMessage.hpp"

struct DataMessage : BCastMessage
{
  DataMessage(string _data);
  
  string serialize();
  void display();

  const string data;
};


#endif
