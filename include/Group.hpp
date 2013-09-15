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

  void remove_peer(shared_ptr<Peer> peer);

  void update_neighbours(PeerMap& predecessors, PeerMap& successors,
                         shared_ptr<Peer> local_peer);

  void display_rings();

  const string get_id() {
    return id;
  }

private:

  PeerMap peers;
  string id;

  // if group is local, those are group rings
  // otherwise, those are channel rings
  Ring rings[RINGS_NB];
};


#endif
