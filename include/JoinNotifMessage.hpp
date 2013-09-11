#ifndef JOIN_NOTIF_MESSAGE_H
#define JOIN_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct JoinNotifMessage : Message
{
  JoinNotifMessage(string _id, string _pub_k, string _ip, unsigned short _port);
  
  string serialize();
  void display();

  const string id;
  const string pub_k;
  const string ip;
  const unsigned short port;

private:
};


#endif
