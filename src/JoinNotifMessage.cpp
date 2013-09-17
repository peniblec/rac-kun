#include <iostream>

#include "JoinNotifMessage.hpp"
#include "Utils.hpp"

JoinNotifMessage::JoinNotifMessage(bool _CHANNEL,
                                   string _group_id, string _peer_id, string _key,
                                   string _ip, unsigned short _port)
  : BCastMessage(MESSAGE_TYPE_JOIN_NOTIF),
    CHANNEL(_CHANNEL),
    group_id(_group_id), peer_id(_peer_id), pub_k(_key), ip(_ip), port(_port)
{
}

string JoinNotifMessage::serialize()
{
  string s = Message::serialize();
  s.append(BCAST_MARKER);
  s.push_back( CHANNEL ? 1 : 0 );
  s.append(peer_id);
  s.append(pub_k);
  s.append(group_id);

  s.append(ip);
  s.push_back(':');
  s.append( itos(port) );
  
  return s;
}

void JoinNotifMessage::display()
{
  Message::display();
  cout << "\tGroup ID: ";
  display_chars(group_id, 10);
  cout << endl;
  cout << "\tPeer ID: " << peer_id << endl;
  cout << "\tPublic key: " << pub_k << endl;
  cout << "\tIP: " << ip << endl;
  cout << "\tPort: " << port << endl;
}
