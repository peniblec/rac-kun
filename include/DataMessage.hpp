#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

class DataMessage : public Message
{
public:
  DataMessage(string _data);
  
  string serialize();
  void display();

  const string get_data() {
    return data;
  }

private:
  string data;
};


#endif
