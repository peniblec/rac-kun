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
#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"

struct JoinMessage : Message // a JOIN request, sent by a node wishing to enter
                             // the system
{
  JoinMessage(string _id, string _pub_k, unsigned short _port);
  
  string serialize();
  void display();

  const string id; // the new peer's unique ID
  const string pub_k; // (placeholder) the new peer's public ID key
  const unsigned short port; // the new peer's listening port
};


#endif
