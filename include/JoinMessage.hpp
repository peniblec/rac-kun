#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct JoinMessage : Message
{
  JoinMessage(string _id, string _pub_k, unsigned short _port);
  
  string serialize();
  void display();

  const string id;
  const string pub_k;
  const unsigned short port;
};


#endif
