#ifndef RING_H
#define RING_H

#include <boost/shared_ptr.hpp>
#include <map>

#include "Config.hpp"
#include "Peer.hpp"

using namespace boost;

struct PeerNotFoundException : public std::exception {
  const char* what() const throw()
  { return "Could not find peer in this ring"; }
};

class Ring {

public:

  Ring(int _index = 0);

  string create_key(string id);
  void add_peer(shared_ptr<Peer> p);

  shared_ptr<Peer> get_successor(shared_ptr<Peer> p);
  shared_ptr<Peer> get_predecessor(shared_ptr<Peer> p);

  void display();  

private:
  typedef map<string, shared_ptr<Peer> > RingMap;

  RingMap::iterator find_peer(string id);

  int index;

  RingMap ring;
};


#endif
