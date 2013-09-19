#include <boost/asio.hpp>
#include <cryptopp/sha.h>
#include <sstream>
#include <sys/time.h>

#include "Utils.hpp"

#include "DataMessage.hpp"
#include "JoinMessage.hpp"
#include "JoinNotifMessage.hpp"
#include "JoinAckMessage.hpp"
#include "ReadyMessage.hpp"
#include "ReadyNotifMessage.hpp"

using boost::asio::ip::tcp;


shared_ptr<Peer> create_local_peer()
{
  // create Peer with dummy socket
  shared_ptr<tcp::socket> null_ptr;
  shared_ptr<Peer> local_peer(new Peer(null_ptr, true));

  // dummy ID generation
  // randomize hostname to allow several nodes on the same machine

  int non_random = 13;
  char name[ID_LENGTH + 1];
  name[ID_LENGTH] = '\0';

  gethostname(name, non_random);

  srand(time(NULL));
  for (uint i=non_random; i<ID_LENGTH; i++) {
    name[i] = (char)('0'+(rand() % 10));
  }
  string id(name);

  // /end dummy ID generation

  local_peer->init( id, "AAAAA" ); // "AAAAA" = dummy public ID key

  return local_peer;
}

void parse_input(string& input, string& command, string& argument)
{
  int space = input.find(' ');

  command = input.substr(0, space);
  argument = input.substr(space+1, input.size());
}

template<typename T> string itos(T t)
{
  stringstream ss;
  ss << t;
  string ret = ss.str();
  return ret;
}
template string itos<unsigned short>(unsigned short us);
template string itos<int>(int i);
template string itos<long>(long l);
template string itos<long long>(long long ll);

Message* parse_message(string msg)
{
  Message::Type msg_type = (Message::Type)msg[0];

  Message* m;
  
  unsigned int offset = 1 + MSG_STAMP_LENGTH; // offset before useful info

  switch (msg_type) {
  case MESSAGE_TYPE_JOIN:
    {
      string id(msg, offset, ID_LENGTH);
      offset+=ID_LENGTH;
      string pub_k(msg, offset, KEY_LENGTH);
      offset+=KEY_LENGTH;
      string endpoint(msg, offset);

      unsigned short port;
      istringstream(endpoint) >> port;

      m = new JoinMessage(id, pub_k, port);
    }
    break;
  case MESSAGE_TYPE_JOIN_NOTIF:
    {
      string bcast_marker(msg, offset, BCAST_MARKER_LENGTH);
      offset+=BCAST_MARKER_LENGTH;

      bool channel_flag( msg[offset] );
      offset+=1;

      string peer_id(msg, offset, ID_LENGTH);
      offset+=ID_LENGTH;
      
      string pub_k(msg, offset, KEY_LENGTH);
      offset+=KEY_LENGTH;

      string group_id(msg, offset, GROUP_ID_LENGTH);
      offset+=GROUP_ID_LENGTH;

      string endpoint(msg, offset);
      int colon = endpoint.find(':');
      string ip( endpoint.substr(0, colon) );
      unsigned short port;
      istringstream ( endpoint.substr( colon+1, endpoint.size() ) ) >> port;
 
      m = new JoinNotifMessage(channel_flag, group_id, peer_id, pub_k,
                               ip, port);
      ((JoinNotifMessage*)m)->BCAST_MARKER = bcast_marker;
    }
    break;
  case MESSAGE_TYPE_JOIN_ACK:
    {
      string peer_id(msg, offset, ID_LENGTH);
      offset+=ID_LENGTH;
      string pub_k(msg, offset, KEY_LENGTH);
      offset+=KEY_LENGTH;
      string group_id(msg, offset, GROUP_ID_LENGTH);

      m = new JoinAckMessage(group_id, peer_id, pub_k);
    }
    break;
  case MESSAGE_TYPE_READY:
    {
      m = new ReadyMessage();
    }
    break;
  case MESSAGE_TYPE_READY_NOTIF:
    {
      m = new ReadyNotifMessage();
    }
    break;
  case MESSAGE_TYPE_DATA:
    {
      string bcast_marker(msg, offset, BCAST_MARKER_LENGTH);
      offset+=BCAST_MARKER_LENGTH;
      string data(msg, offset);

      m = new DataMessage(data);
      ((DataMessage*)m)->BCAST_MARKER = bcast_marker;
    }
    break;
  default:
    throw MessageParseException();
  }

  // retrieve stamp
  string stamp(msg, 1, MSG_STAMP_LENGTH);
  m->stamp = stamp;

  return m;
}

string make_hash(string input)
{
  CryptoPP::SHA1 hash;

  byte digest[CryptoPP::SHA1::DIGESTSIZE];

  hash.CalculateDigest( digest, (byte*) input.c_str(), input.size() );
  // to make it human-readable:
  // http://www.cryptopp.com/wiki/HexEncoder

  string output((char*)digest, sizeof(digest));
  // using the (char*, size_t) constructor:
  // plain (char*) constructor will truncate after the first null byte

  return output;
}

long long milliseconds_since_epoch()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long long now = ((long long)tv.tv_sec)*1000 + tv.tv_usec/1000;
  return now;
}

void display_chars(string s, unsigned int n)
{
  n = ( n <= s.size() ? n : s.size() );

  for (unsigned int i = 0; i<n; i++)
    cout << (int) ((unsigned char) s[i])
         << (i+1==s.size() ? "." : i+1==n ? "..." : "-");
}
