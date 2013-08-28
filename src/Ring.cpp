#include "Ring.hpp"
#include "Utils.hpp"

Ring::Ring(int _index)
  : index(_index)
{
}

string Ring::create_key(string id)
{
  string concat = id;
  concat.append(itos(index));

  string key = make_hash(concat);

  return key;
}

void Ring::add_peer(shared_ptr<Peer> p)
{
  string key = create_key( p->get_id() );
  ring[key] = p;
}

void Ring::remove_peer(shared_ptr<Peer> p)
{
  string key = create_key( p->get_id() );
  ring.erase( key );
}

shared_ptr<Peer> Ring::get_successor(shared_ptr<Peer> p)
{
  RingMap::iterator it = find_peer( p->get_id() );
  
  if ( it!= ring.end() && ring.size()>1 ) {

    it++;
    return ( it != ring.end() ? it->second : ring.begin()->second );
  }
  else 
    throw PeerNotFoundException();
}

shared_ptr<Peer> Ring::get_predecessor(shared_ptr<Peer> p)
{
  RingMap::iterator it = find_peer( p->get_id() );
 
  if ( it!= ring.end() && ring.size()>1 )
    return ( it != ring.begin() ? (--it)->second : ring.rbegin()->second );

  else 
    throw PeerNotFoundException();
}

void Ring::display()
{
  RingMap::iterator it;

  cout << "Ring index: " << index << endl;
  for (it=ring.begin(); it!=ring.end(); it++) {

    cout << it->second->get_id() << " " 
         << PeerStateNames[it->second->get_state()] << " with key: " << endl << "\t";
    for (uint n=0; n< (it->first.size()); n++) {
      cout << (int) ((unsigned char) it->first[n]) << (n+1==it->first.size() ? "" : "-");
    }
    cout << " [" << it->first.size() << "]" << endl;
  }
}

Ring::RingMap::iterator Ring::find_peer(string id) // TODO: move throw here?
{
  string key = create_key(id);
  RingMap::iterator it = ring.find(key);
  return it;
}
