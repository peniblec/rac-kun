#include <sstream>

#include "Utils.hpp"

#include "JoinMessage.hpp"
#include "JoinNotifMessage.hpp"
#include "JoinAckMessage.hpp"
#include "ReadyMessage.hpp"
#include "ReadyNotifMessage.hpp"


void parse_input(string& input, string& command, string& argument)
{
  int space = input.find(' ');
  int l;

  char buf[input.size()];

  l = input.copy(buf, space, 0);
  buf[l] = '\0';
  command=string(buf);

  l = input.copy(buf, input.size() - space, space+1);
  buf[l] = '\0';
  argument=string(buf);
}

string itos(int i)
{
  stringstream ss;
  ss << i;
  string ret = ss.str();
  return ret;
}

Message* parse_message(string msg)
{
  Message::Type msg_type = (Message::Type)msg[0];

  switch (msg_type) {
  case MESSAGE_TYPE_JOIN:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);

      JoinMessage* m = new JoinMessage(id, pub_k);
    
      return m;
    }
  case MESSAGE_TYPE_JOIN_NOTIF:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);
      string ip(msg, JOIN_NOTIF_IP_OFFSET);
      
      JoinNotifMessage* m = new JoinNotifMessage(id, pub_k, ip);

      return m;
    }
  case MESSAGE_TYPE_JOIN_ACK:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);

      JoinAckMessage* m = new JoinAckMessage(id, pub_k);
    
      return m;
    }
  case MESSAGE_TYPE_READY:
    {
      ReadyMessage* m = new ReadyMessage();
    
      return m;
    }
  case MESSAGE_TYPE_READY_NOTIF:
    {
      ReadyNotifMessage* m = new ReadyNotifMessage();

      return m;
    }

  default:
    throw MessageParseException();
  }
      
}
