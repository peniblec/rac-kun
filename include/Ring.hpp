#ifndef RING_H
#define RING_H

#include <cryptopp/sha.h>
#include <map>

#include "Config.hpp"
#include "Peer.hpp"

struct PeerNotFoundException : public std::exception {
  const char* what() const throw()
  { return "Could not find peer in this ring"; }
};

class Ring {

public:

  Ring(int _index);

  void add_peer(shared_ptr<Peer> p);

  shared_ptr<Peer> get_successor(shared_ptr<Peer> p);
  shared_ptr<Peer> get_predecessor(shared_ptr<Peer> p);


private:
  int index;
  CryptoPP::SHA1 hash;

  map<string, shared_ptr<Peer> > ring;
};


#endif
