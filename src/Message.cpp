#include <iostream>

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
  // stamp recipy: hash( author_id || timestamp )
  string input = peer_id;

  input.append( itos(milliseconds_since_epoch()) );
  input.push_back( (char)type );

  stamp = make_hash(input);
}

bool Message::is_broadcast()
{
  return false;
}

void Message::display()
{
  cout << MessageTypeNames[ type ] << endl;
  display_chars( stamp, 10 );
  cout << endl;
}

string Message::serialize()
{
  // typical message format:
  // byte[0]: type
  // byte[1-20]: stamp
  // byte[21+]: message-specific info
  string s;
  s.push_back(type);
  s.append(stamp);
  return s;
}

