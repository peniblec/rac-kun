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

#include "JoinAckMessage.hpp"
#include "Utils.hpp"

JoinAckMessage::JoinAckMessage(string _group_id, string _peer_id, string _key)
  : Message(MESSAGE_TYPE_JOIN_ACK),
    group_id(_group_id), peer_id(_peer_id), pub_k(_key)
{
}

string JoinAckMessage::serialize()
{
  string s = Message::serialize();
  s.append(peer_id);
  s.append(pub_k);
  s.append(group_id);
  return s;
}

void JoinAckMessage::display()
{
  Message::display();
  cout << "\tGroup ID: ";
  display_chars(group_id, 10);
  cout << endl;
  cout << "\tPeer ID: " << peer_id << endl;
  cout << "\tPublic key: " << pub_k << endl;
}
