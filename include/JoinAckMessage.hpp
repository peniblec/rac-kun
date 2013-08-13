#ifndef JOIN_ACK_MESSAGE_H
#define JOIN_ACK_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

class JoinAckMessage : public Message
{
public:
  JoinAckMessage(string _id, string _pub_k);
  
  string serialize();
  void display();

  const string get_id() {
    return id;
  }

  const string get_key() {
    return pub_k;
  }
  
private:
  string id;
  string pub_k;
};


#endif
