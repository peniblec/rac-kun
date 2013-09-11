#ifndef UTILS_H
#define UTILS_H

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <string>

#include "Config.hpp"
#include "Message.hpp"
#include "Peer.hpp"

using namespace boost;

#ifdef DEBUG_ON
#define DEBUG(x) do { std::cerr << "DBG: " << x << std::endl; } while (0)
#else
#define DEBUG(x)
#endif

typedef map<string, shared_ptr<Peer> > PeerMap;
// associates ID with peer

shared_ptr<Peer> create_local_peer();

void parse_input(string& input, string& command, string& argument);

string itos(int i);
string ltos(long l);

Message* parse_message(string msg);

string make_hash(string input);

#endif
