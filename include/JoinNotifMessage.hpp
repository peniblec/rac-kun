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
#ifndef JOIN_NOTIF_MESSAGE_H
#define JOIN_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "BCastMessage.hpp"

struct JoinNotifMessage : BCastMessage // a notification containing all the info
                                       // a node needs to contact the new peer
                                       // and add it to its view
{
  JoinNotifMessage(bool _CHANNEL,
                   string _group_id, string _peer_id, string _pub_k,
                   string _ip, unsigned short _port);
  
  string serialize();
  void display();

  const bool CHANNEL; // a flag indicating where in the joining process the new
                      // peer is:
                      // - if false, the peer is only joining its own group, and
                      //   any node who is not in this group should not add the
                      //   peer to their view right away
                      // - if true, the peer has completed the JOIN procedure
                      //   inside its own group, and is now joining the channels

  const string group_id; // the ID of the group this peer should join (all nodes
                         // should use peer_id to check whether this is the
                         // correct group)

  const string peer_id; // the peer's unique ID

  const string pub_k; // (placeholder) the peer's public ID key

  const string ip; // the IP address to use to connect to the peer
  const unsigned short port; // the new peer's listening port

};


#endif
