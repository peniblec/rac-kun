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
#include "Ring.hpp"


Ring::Ring(int _index)
  : index(_index)
{
}

string Ring::create_key(string id)
{
  // key format: hash( peer ID || ring index )
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
  PeerMap::iterator it = find_peer(p);

  if ( it!=ring.end() )
    ring.erase(it);
}

shared_ptr<Peer> Ring::get_successor(shared_ptr<Peer> p)
{
  PeerMap::iterator it = find_peer(p);
  
  if ( it!= ring.end() && ring.size()>1 ) {
    // check that it is not the map's last peer
    return ( ++it != ring.end() ? it->second : ring.begin()->second );
  }
  else 
    throw PeerNotFoundException();
}

shared_ptr<Peer> Ring::get_predecessor(shared_ptr<Peer> p)
{
  PeerMap::iterator it = find_peer(p);
 
  if ( it!= ring.end() && ring.size()>1 )
    // check that it is not the map's first peer
    return ( it != ring.begin() ? (--it)->second : ring.rbegin()->second );

  else 
    throw PeerNotFoundException();
}

void Ring::display()
{
  PeerMap::iterator it;

  cout << "Ring index: " << index << endl;
  for (it=ring.begin(); it!=ring.end(); it++) {

    cout << it->second->get_id() << " "
         << PeerStateNames[it->second->get_state()] << endl;
  }
}

PeerMap::iterator Ring::find_peer(shared_ptr<Peer> p)
{
  string key = create_key( p->get_id() );
  PeerMap::iterator it = ring.find(key);
  return it;
}
