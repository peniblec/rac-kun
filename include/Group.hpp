#ifndef GROUP_H
#define GROUP_H

#include <map>

#include "Config.hpp"
#include "Peer.hpp"
#include "Ring.hpp"
#include "Utils.hpp"

using namespace boost;

class Group {

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

  PeerMap predecessors;
  PeerMap successors;

  Ring rings[RINGS_NB]; // if group is local, those are group rings;
                        // otherwise, those are the channel rings shared between
                        // this group and the local one

};


#endif
