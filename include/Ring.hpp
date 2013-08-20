#ifndef RING_H
#define RING_H

#include <boost/shared_ptr.hpp>
#include <cryptopp/sha.h>
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

  void add_peer(shared_ptr<Peer> p);

  shared_ptr<Peer> get_successor(shared_ptr<Peer> p);
  shared_ptr<Peer> get_predecessor(shared_ptr<Peer> p);

  void display();  

private:
  typedef map<string, shared_ptr<Peer> > RingMap;

  int index;
  CryptoPP::SHA1 hash;

  RingMap ring;
};


#endif
