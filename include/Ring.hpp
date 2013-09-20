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
#ifndef RING_H
#define RING_H

#include <boost/shared_ptr.hpp>
#include <map>

#include "Config.hpp"
#include "Peer.hpp"
#include "Utils.hpp"

using namespace boost;

struct PeerNotFoundException : public std::exception {
  const char* what() const throw()
  { return "Could not find peer in this ring"; }
};

class Ring // a structure of peers sorted according a hash function
{

public:

  Ring(int _index = 0);

  /* add_peer:
     - add a peer to the structure

     - p: the peer to add
   */
  void add_peer(shared_ptr<Peer> p);

  /* remove_peer:
     - attempt to find the peer in the structure; if found, remove him

     - p: the peer to remove
   */
  void remove_peer(shared_ptr<Peer> p);

  /* get_successor:
     - find the peer indexed right after the specified peer

     - p: the peer whose successor should be returned;
     - throws if p is not found in the structure, or if it contains only one
       peer 
   */
  shared_ptr<Peer> get_successor(shared_ptr<Peer> p);

  /* get_predecessor:
     - find the peer indexed right before the specified peer

     - p: the peer whose predecessor should be returned;
     - throws if p is not found in the structure, or if it contains only one
       peer 
   */
  shared_ptr<Peer> get_predecessor(shared_ptr<Peer> p);

  /*  display:
      - show every peer this ring contains, sorted according to their hash
   */
  void display();

private:
  /* create_key:
     - called when a peer needs to be added/found in the structure
     - generate a hash that may be used to index the peer in this ring

     - id: the peer's unique ID
     - returns the hash used to sort the peer
   */
  string create_key(string id);

  /* find_peer:
     - attempt to find the specified peer in the structure

     - p: the peer to find
     - returns an iterator to the peer (which will be ring.end() if p is not
       present in the ring)
   */
  PeerMap::iterator find_peer(shared_ptr<Peer> p);

  int index; // this ring's index, used to create hashes which sort peers in a
             // random order
  PeerMap ring; // contains this ring's peers, sorted randomly according to a
                // hash function
};


#endif
