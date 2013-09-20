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
#ifndef GROUP_H
#define GROUP_H

#include <map>

#include "Config.hpp"
#include "Peer.hpp"
#include "Ring.hpp"
#include "Utils.hpp"

using namespace boost;

class Group // a subset of the peers composing the system, organized in rings
{

public:

  Group(string _id);

  /* add_peer:
     - add a peer to this group and to the rings
     - calls add_to_rings

     - peer: a peer who belongs in this group
   */
  void add_peer(shared_ptr<Peer> peer);

  /* add_to_rings:
     - add a peer to the rings only
     - used to build channels: half of the peers participating in a channel
       belong to one of the two groups

     - peer: a peer who does not necessarily belong to the group
   */
  void add_to_rings(shared_ptr<Peer> peer);

  /* add_to_rings:
     - add a whole range of peers to the ring
     - used to build channels quickly between this group and the local group

     - other_group: the group with the other half of the peers we need to
       constitute the channel
   */
  void add_to_rings(shared_ptr<Group> other_group);

  /* remove_peer:
     - remove this peer from the rings and from the group itself

     - peer: the peer to remove
  */
  bool remove_peer(shared_ptr<Peer> peer);

  /* update_neighbours:
     - clear and rebuild predecessors and successors

     - local_peer: the reference peer whose predecessors and successors we are
       trying to find
   */
  void update_neighbours(shared_ptr<Peer> local_peer);

  /* display_rings:
     - display all peers belonging to this group, then the composition of the
       various rings (see Ring)
   */
  void display_rings();

  const PeerMap get_predecessors() {
    return predecessors;
  }

  const PeerMap get_successors() {
    return successors;
  }

  const string get_id() {
    return id;
  }

private:

  PeerMap peers; // the peers belonging to this group
  string id;

  PeerMap predecessors; // the local peer's successors in the various
                        // rings, sorted by their ID
  PeerMap successors; // resp. predecessors

  Ring rings[RINGS_NB]; // if group is local, those are group rings;
                        // otherwise, those are the channel rings shared between
                        // this group and the local one

};


#endif
