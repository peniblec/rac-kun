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
#ifndef MESSAGE_H
#define MESSAGE_H

#include <exception>

#include "Config.hpp"

struct MessageParseException : public std::exception {
  const char* what() const throw()
  { return "Could not parse message"; }
};

struct Message // a message structured according to the RAC protocol
{
  typedef MessageType Type;

  virtual ~Message();

  /* serialize:
     - get a string containing the message's information, ready to be sent
       across the network
   */
  virtual string serialize();

  /* display:
     - show the message's content
   */
  virtual void display();

  /* make_stamp:
     - create a unique stamp to allow peers to recognize this message when it's
       broadcast

     - peer_id: the original emitter of the message
   */
  void make_stamp(string peer_id);

  /* is_broadcast:
     - returns whether this message should be passed along the rings or if it's
       just a one-time signal

     - by default, returns false
   */
  virtual bool is_broadcast();

  const Type type; // type of message, used for serialization (see individual
                   // classes for details)
  string stamp; // unique ID used to recognize the message after broadcasting it

protected:
  Message(Type _type);
};


#endif
