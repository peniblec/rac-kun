#ifndef JOIN_NOTIF_MESSAGE_H
#define JOIN_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

class JoinNotifMessage : public Message
{
public:
  JoinNotifMessage(string _id, string _pub_k, string _ip, unsigned short _port);
  
  string serialize();
  void display();

  const string get_id() {
    return id;
  }

  const string get_key() {
    return pub_k;
  }
  
  const string get_ip() {
    return ip;
  }

  const unsigned short get_port() {
    return port;
  }

private:
  string id;
  string pub_k;
  string ip;
  unsigned short port;
};


#endif
