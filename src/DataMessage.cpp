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

#include "DataMessage.hpp"

DataMessage::DataMessage(string _data)
  : BCastMessage(MESSAGE_TYPE_DATA), data(_data)
{
}

string DataMessage::serialize()
{
  string s = Message::serialize();
  s.append(BCAST_MARKER);
  s.append(data);
  return s;
}

void DataMessage::display()
{
  Message::display();
  cout << "\tData: " << data << endl;
}
