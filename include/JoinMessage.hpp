#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

class JoinMessage : public Message
{
public:
  JoinMessage(string _id, string _pub_k, unsigned short _port);
  
  string serialize();
  void display();

  const string get_id() {
    return id;
  }

  const string get_key() {
    return pub_k;
  }
  
  const unsigned short get_port() {
    return port;
  }

private:
  string id;
  string pub_k;
  unsigned short port;
};


#endif
