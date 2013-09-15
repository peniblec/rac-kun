#ifndef JOIN_ACK_MESSAGE_H
#define JOIN_ACK_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct JoinAckMessage : Message
{
  JoinAckMessage(string _group_id, string _peer_id, string _pub_k);
  
  string serialize();
  void display();

  const string group_id;
  const string peer_id;
  const string pub_k;
};


#endif
