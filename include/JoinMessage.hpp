#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct JoinMessage : Message // a JOIN request, sent by a node wishing to enter
                             // the system
{
  JoinMessage(string _id, string _pub_k, unsigned short _port);
  
  string serialize();
  void display();

  const string id; // the new peer's unique ID
  const string pub_k; // (placeholder) the new peer's public ID key
  const unsigned short port; // the new peer's listening port
};


#endif
