#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

class JoinMessage : public Message
{
public:
  JoinMessage();
  JoinMessage(string raw);

private:
  string id;
  string pub_k;
};


#endif
