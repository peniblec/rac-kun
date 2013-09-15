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
  shared_ptr<tcp::socket> null_ptr;
  shared_ptr<Peer> local_peer(new Peer(null_ptr, true));

  char name[ID_LENGTH + 1];
  name[ID_LENGTH] = '\0';
  // gethostname is not guaranteed to add \0 if it needs to truncate host name
  gethostname(name, sizeof(name) -1);

  string id(name);

  local_peer->init( id, "AAAAA" );

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
  
  // DEBUG

  // if (msg_type == MESSAGE_TYPE_JOIN_NOTIF) {

  //   DEBUG("Receiving a " << MessageTypeNames[msg_type] << " of size " << msg.size());

  //   for (uint n=MSG_STAMP_OFFSET; n< (MSG_STAMP_OFFSET+MSG_STAMP_LENGTH); n++)
  //     cout << (int) ((unsigned char) msg[n]) << '-';
  //   cout << endl;
  // }
  // // /DEBUG

  Message* m;

  // try{
  
  switch (msg_type) {
  case MESSAGE_TYPE_JOIN:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);

      string endpoint(msg, JOIN_REQUEST_PORT_OFFSET);

      unsigned short port;
      istringstream(endpoint) >> port;

      m = new JoinMessage(id, pub_k, port);
    }
    break;
  case MESSAGE_TYPE_JOIN_NOTIF:
    {
      string peer_id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);
      string group_id(msg, JOIN_NOTIF_GROUP_ID_OFFSET, GROUP_ID_LENGTH);

      string endpoint(msg, JOIN_NOTIF_ENDPOINT_OFFSET);
      int colon = endpoint.find(':');

      string ip( endpoint.substr(0, colon) );
      unsigned short port;
      istringstream ( endpoint.substr( colon+1, endpoint.size() ) ) >> port;
 
      m = new JoinNotifMessage(group_id, peer_id, pub_k, ip, port);
    }
    break;
  case MESSAGE_TYPE_JOIN_ACK:
    {
      string peer_id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);
      string group_id(msg, JOIN_NOTIF_GROUP_ID_OFFSET, GROUP_ID_LENGTH);

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
      string data(msg, DATA_MESSAGE_OFFSET);

      m = new DataMessage(data);
    }
    break;
  default:
    throw MessageParseException();
  }
  string stamp(msg, MSG_STAMP_OFFSET, MSG_STAMP_LENGTH);
  m->stamp = stamp;

  return m;
  // }
  // catch (std::exception& e) {
  //   cout << e.what() << endl;
  // // DEBUG

  // if (msg_type < MESSAGE_TYPE_END) {

  //   DEBUG("Receiving a " << MessageTypeNames[msg_type] << " of size " << msg.size());

  //   for (uint n=0; n< msg.size() ; n++)
  //     cout << (int) ((unsigned char) msg[n]) << '-';
  //   cout << endl;
  // }
  // throw MessageParseException();

  // // // /DEBUG    

  // }
}

string make_hash(string input)
{
  CryptoPP::SHA1 hash;

  byte digest[CryptoPP::SHA1::DIGESTSIZE];

  hash.CalculateDigest( digest, (byte*) input.c_str(), input.size() );
  // http://www.cryptopp.com/wiki/HexEncoder

  string output((char*)digest, sizeof(digest));
  // using the (char*, size_t) constructor:
  // (char*) constructor will truncate after the first null byte

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
    cout << (int) ((unsigned char) s[i]) << (i+1==s.size() ? "." : i+1==n ? "..." : "-");
}
