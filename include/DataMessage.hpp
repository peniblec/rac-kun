#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include "Config.hpp"

#include "BCastMessage.hpp"

struct DataMessage : BCastMessage // message used to pass cleartext
                                  // information along the rings
{
  DataMessage(string _data);
  
  string serialize();
  void display();

  const string data; // a stream of bytes (for now, displayed as cleatext)
};


#endif
