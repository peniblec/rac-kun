#ifndef JOIN_NOTIF_MESSAGE_H
#define JOIN_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "BCastMessage.hpp"

struct JoinNotifMessage : BCastMessage
{
  JoinNotifMessage(bool _CHANNEL,
                   string _group_id, string _peer_id, string _pub_k,
                   string _ip, unsigned short _port);
  
  string serialize();
  void display();

  const bool CHANNEL;
  const string group_id;
  const string peer_id;
  const string pub_k;
  const string ip;
  const unsigned short port;

};


#endif
