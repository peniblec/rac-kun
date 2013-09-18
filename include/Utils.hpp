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


typedef map<string, shared_ptr<Peer> > PeerMap; // Associates a string (ID,
                                                // hash, ...) with a Peer.

/* create_local_peer:
   - called at startup
   - returns a dummy peer, generates the local ID
 */
shared_ptr<Peer> create_local_peer();

/* parse_input:
   - called by UI
   - separates command and argument

   - input: the string typed by the user
   - command: the part before the first space, supposedly, a valid command
   - argument: the part after the first space, supposedly, a valid argument
 */
void parse_input(string& input, string& command, string& argument);

/* itos:
   - converts different sizes of integers into std::string

   - t: the variable to convert to string. itos() was written with integer types
        in mind and uses stringstreams to generate a string, so it may work with
        other types (for example, stream buffers). This whole function could be
        replaced with boost::lexical_cast<string>, but itos() is shorter to
        write...
 */
template<typename T> string itos(T t);

/* parse_message:
   - called mainly when receiving a message
   - creates a Message structure, fills it with the message's content, and
     returns a pointer to it
   - throws when parsing fails (either because the type is not recognized
     (MessageParseException) or because we tried to read out of bounds of msg
     (std::out_of_range))

   - msg: the raw message
   - returns a pointer to a derived type of Message
 */
Message* parse_message(string msg);

/* make_hash:
   - calls Crypto++ to make a unique hash of the given string (using SHA1)

   - input: the string to hash
   - returns a string containing CryptoPP::SHA1::DIGESTSIZE characters
 */
string make_hash(string input);

/* milliseconds_since_epoch:
   - used at various places to get a unique time stamp

   - returns the number of milliseconds since epoch
     (long long integers going up to 2^63-1 (minimum), timestamps will stop
     being  unique at some point in April 2262)
 */
long long milliseconds_since_epoch();

/* display_chars:
   - displays the numeric value for the first n characters of a std::string
   - used to display IDs, for examples

   - s: the string of characters to display
   - n: the maximum amount of characters to show
 */
void display_chars(string s, unsigned int n);

#endif
