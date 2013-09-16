#include "Group.hpp"

Group::Group(string _id)
  : id(_id)
{
  for (int i=0; i<RINGS_NB; i++)
    rings[i] = Ring(i);
}

void Group::add_peer(shared_ptr<Peer> peer)
{
  peers.insert(pair<string, shared_ptr<Peer> >(peer->get_id(), peer));

  for (int i=0; i<RINGS_NB; i++) {

    rings[i].add_peer(peer);
  }
}

void Group::remove_peer(shared_ptr<Peer> peer)
{
  for (int i=0; i<RINGS_NB; i++)
    rings[i].remove_peer(peer);
  // TODO: remove peer from peers
}

void Group::update_neighbours(PeerMap& predecessors, PeerMap& successors,
                       shared_ptr<Peer> local_peer)
{
  PeerMap preds, succs;
  shared_ptr<Peer> p;
  try {
    for (int i=0; i<RINGS_NB; i++) {
    
      p = rings[i].get_predecessor( local_peer );
      preds.insert( pair<string, shared_ptr<Peer> >( p->get_id(), p ) );

      p = rings[i].get_successor( local_peer );
      succs.insert( pair<string, shared_ptr<Peer> >( p->get_id(), p ) );
    }
    predecessors = preds;
    successors = succs;
  }
  catch (PeerNotFoundException& e) { 
    DEBUG("Group::update_neighbours: " << e.what());
    predecessors.clear();
    successors.clear();
  }
}

void Group::display_rings()
{
  for (int i=0; i<RINGS_NB; i++) {
    
    rings[i].display();
    cout << endl;
  }
}
