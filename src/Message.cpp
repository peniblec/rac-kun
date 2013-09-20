/*

  RAC-kun - junior version of RAC, a freerider-resilient, scalable, anonymous
  communication protocol conceived by researchers from CNRS and LIG.
  Copyright (C) 2013  Kévin Le Gouguec

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see [http://www.gnu.org/licenses/].

*/
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

