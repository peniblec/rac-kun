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

#include "JoinMessage.hpp"
#include "Utils.hpp"

JoinMessage::JoinMessage(string _id, string _key, unsigned short _port)
  : Message(MESSAGE_TYPE_JOIN), id(_id), pub_k(_key), port(_port)
{
}

string JoinMessage::serialize()
{
  string s = Message::serialize();
  s.append(id);
  s.append(pub_k);
  s.append( itos(port) );
  return s;
}

void JoinMessage::display()
{
  Message::display();
  cout << "\tPeer ID: " << id << endl;
  cout << "\tPublic key: " << pub_k << endl;
  cout << "\tPort: " << port << endl;
}
