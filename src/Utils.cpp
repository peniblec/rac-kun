#include <sstream>

#include "Message.hpp"
#include "Utils.hpp"



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

  DEBUG("Type found: " << msg_type);
  
  switch (msg_type) {
  case MESSAGE_TYPE_JOIN:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);

      JoinMessage* m = new JoinMessage(id, pub_k);
    
      return m;
    }
  default:
    throw MessageParseException();
  }
      
}
