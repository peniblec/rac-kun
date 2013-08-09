#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

class JoinMessage : public Message
{
public:
  // JoinMessage();
  // JoinMessage(string raw);
  JoinMessage(string _id, string _pub_k);
  
  string serialize();
  void display();

private:
  string id;
  string pub_k;
};


#endif
