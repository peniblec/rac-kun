#include "Ring.hpp"
#include "Utils.hpp"

Ring::Ring(int _index)
  : index(_index)
{
}

void Ring::add_peer(shared_ptr<Peer> p)
{
  string concat = p->get_id();
  concat.append(itos(index));

  byte digest[CryptoPP::SHA1::DIGESTSIZE+1];
  // TODO: make sure ID_SIZE <= DIGESTSIZE
  hash.CalculateDigest( digest, (byte*) concat.c_str(), concat.length() );
  // http://www.cryptopp.com/wiki/HexEncoder

  digest[CryptoPP::SHA1::DIGESTSIZE] = '\0';

  string key((char*)digest);

  ring[key] = p;
}

shared_ptr<Peer> Ring::get_successor(shared_ptr<Peer> p)
{
  RingMap::iterator it;
  it = ring.find( p->get_id() );

  if ( it!= ring.end() ) {

    it++;
    return ( it != ring.end() ? it->second : ring.begin()->second );
  }
  else 
    throw PeerNotFoundException();
}

shared_ptr<Peer> Ring::get_predecessor(shared_ptr<Peer> p)
{
  map<string, shared_ptr<Peer> >::iterator it;
  it = ring.find( p->get_id() );

  if ( it!= ring.end() )
    return ( it != ring.begin() ? (--it)->second : ring.rbegin()->second );

  else 
    throw PeerNotFoundException();
}
