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
  add_to_rings(peer);
}

void Group::add_to_rings(shared_ptr<Peer> peer)
{
  for (int i=0; i<RINGS_NB; i++) {

    rings[i].add_peer(peer);
  }
}

void Group::add_to_rings(shared_ptr<Group> other_group)
{
  PeerMap other_peers = other_group->peers;

  for (int i=0; i<RINGS_NB; i++) {

    PeerMap::iterator it;
    for (it=other_peers.begin(); it!=other_peers.end(); it++) {

      rings[i].add_peer(it->second);
    }
  }
}

bool Group::remove_peer(shared_ptr<Peer> peer)
{
  for (int i=0; i<RINGS_NB; i++)
    rings[i].remove_peer(peer);

  return ( peers.erase( peer->get_id() ) > 0 );
}

void Group::update_neighbours(shared_ptr<Peer> local_peer)
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
  cout << "Peers: ";
  for (PeerMap::iterator it = peers.begin(); it!=peers.end(); it++)
    cout << it->first << " ; ";
  cout << endl;
  
  for (int i=0; i<RINGS_NB; i++) {
    
    rings[i].display();
    cout << endl;
  }
}
