#ifndef JOIN_ACK_MESSAGE_H
#define JOIN_ACK_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct JoinAckMessage : Message // an acknowledgement sent by a member of the
                                // system to a joining node, so that it can
                                // build a complete view of the network
{
  JoinAckMessage(string _group_id, string _peer_id, string _pub_k);
  
  string serialize();
  void display();

  const string group_id; // the ID of the group the local peer belongs to, which
                         // the new peer will use to determine what groups exist
                         // out there

  const string peer_id; // the local peer's unique ID

  const string pub_k; // (placeholder) the local peer's public ID key
};


#endif
