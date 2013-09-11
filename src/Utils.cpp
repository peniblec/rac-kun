#include <boost/asio.hpp>
#include <cryptopp/sha.h>
#include <sstream>

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
  // TODO: clean this with substr
  int space = input.find(' ');
  int l;

  char buf[input.size()];

  l = input.copy(buf, space, 0);
  buf[l] = '\0';
  command=string(buf);

  l = input.copy(buf, input.size() - space, space+1);
  buf[l] = '\0';
  argument=string(buf);
}

string itos(int i)
{
  stringstream ss;
  ss << i;
  string ret = ss.str();
  return ret;
}

string ltos(long l)
{
  stringstream ss;
  ss << l;
  string ret = ss.str();
  return ret;
}


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

      string endpoint(msg, JOIN_NOTIF_ENDPOINT_OFFSET);

      unsigned short port;
      istringstream(endpoint) >> port;

      m = new JoinMessage(id, pub_k, port);
    }
    break;
  case MESSAGE_TYPE_JOIN_NOTIF:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);
      string endpoint(msg, JOIN_NOTIF_ENDPOINT_OFFSET);

      int colon = endpoint.find(':');

      string ip( endpoint.substr(0, colon) );
      unsigned short port;
      istringstream ( endpoint.substr( colon+1, endpoint.size() ) ) >> port;
 
      m = new JoinNotifMessage(id, pub_k, ip, port);
    }
    break;
  case MESSAGE_TYPE_JOIN_ACK:
    {
      string id(msg, JOIN_MSG_ID_OFFSET, JOIN_MSG_ID_LENGTH);
      string pub_k(msg, JOIN_MSG_KEY_OFFSET, JOIN_MSG_KEY_LENGTH);

      m = new JoinAckMessage(id, pub_k);
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
