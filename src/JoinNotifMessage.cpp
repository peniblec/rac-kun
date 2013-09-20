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

#include "JoinNotifMessage.hpp"
#include "Utils.hpp"

JoinNotifMessage::JoinNotifMessage(bool _CHANNEL,
                                   string _group_id, string _peer_id, string _key,
                                   string _ip, unsigned short _port)
  : BCastMessage(MESSAGE_TYPE_JOIN_NOTIF),
    CHANNEL(_CHANNEL),
    group_id(_group_id), peer_id(_peer_id), pub_k(_key), ip(_ip), port(_port)
{
}

string JoinNotifMessage::serialize()
{
  string s = Message::serialize();
  s.append(BCAST_MARKER);
  s.push_back( CHANNEL ? 1 : 0 );
  s.append(peer_id);
  s.append(pub_k);
  s.append(group_id);

  s.append(ip);
  s.push_back(':');
  s.append( itos(port) );
  
  return s;
}

void JoinNotifMessage::display()
{
  Message::display();
  cout << "\tGroup ID: ";
  display_chars(group_id, 10);
  cout << endl;
  cout << "\tPeer ID: " << peer_id << endl;
  cout << "\tPublic key: " << pub_k << endl;
  cout << "\tIP: " << ip << endl;
  cout << "\tPort: " << port << endl;
}
