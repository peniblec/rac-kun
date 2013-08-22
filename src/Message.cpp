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
  // TODO: sane timestamp!!
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int now = (tv.tv_sec*1000 + tv.tv_usec/1000);

  input.append( itos(now) );

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
