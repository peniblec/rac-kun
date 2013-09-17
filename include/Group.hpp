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

  void add_peer(shared_ptr<Peer> peer);

  void add_to_rings(shared_ptr<Peer> peer);

  void add_to_rings(shared_ptr<Group> other_group);

  bool remove_peer(shared_ptr<Peer> peer);

  void update_neighbours(shared_ptr<Peer> local_peer);

  const PeerMap get_predecessors() {
    return predecessors;
  }

  const PeerMap get_successors() {
    return successors;
  }

  void display_rings();

  const string get_id() {
    return id;
  }

private:

  PeerMap peers;
  string id;

  PeerMap predecessors;
  PeerMap successors;

  // if group is local, those are group rings
  // otherwise, those are channel rings
  Ring rings[RINGS_NB];
};


#endif
