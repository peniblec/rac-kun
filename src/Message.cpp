#include <iostream>
#include <sys/time.h>

#include "Message.hpp"
#include "Utils.hpp"

Message::Message(Message::Type _type)
  : type(_type)
{
}

Message::~Message()
{
}

void Message::make_stamp(string peer_id)
{
  string input = peer_id;

  struct timeval tv;
  gettimeofday(&tv, NULL);
  long now = (tv.tv_sec*1000 + tv.tv_usec/1000);
  // TODO: check whether long will work on most machines

  input.append( ltos(now) );
  input.push_back( (char)type );

  stamp = make_hash(input);
}

void Message::set_stamp(string _stamp)
{
  stamp = _stamp;
}

const Message::Type Message::get_type()
{
  return type;
}

bool Message::is_broadcast()
{
  return ( type == MESSAGE_TYPE_JOIN_NOTIF
           || type == MESSAGE_TYPE_DATA );
}

string Message::serialize()
{
  string s;
  s.push_back(type);
  s.append(stamp);
  return s;
}

// void Message::display()
// {
//   cout << "Empty message" << endl;
// }
