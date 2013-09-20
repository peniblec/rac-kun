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
#ifndef READY_NOTIF_MESSAGE_H
#define READY_NOTIF_MESSAGE_H

#include "Config.hpp"

#include "Message.hpp"


struct ReadyNotifMessage : Message // a signal sent by a joining node to its
                                   // direct predecessors and successors, to
                                   // indicate that it's ready to be used as
                                   // their predecessor/successor
{
  ReadyNotifMessage();
  
  string serialize();
  void display();
};


#endif
