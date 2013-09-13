#include <iostream>

#include "JoinNotifMessage.hpp"
#include "Utils.hpp"

JoinNotifMessage::JoinNotifMessage(/*string _group_id, */string _peer_id, string _key,
                                   string _ip, unsigned short _port)
  : Message(MESSAGE_TYPE_JOIN_NOTIF),
    /*group_id(_group_id),*/ peer_id(_peer_id), pub_k(_key), ip(_ip), port(_port)
{
}

string JoinNotifMessage::serialize()
{
  string s = Message::serialize();
  s.append(peer_id);
  s.append(pub_k);
  // s.append(group_id);

  s.append(ip);
  s.push_back(':');
  s.append( itos(port) );
  
  return s;
}

void JoinNotifMessage::display()
{
  cout << "Peer with id " << peer_id << " (" << ip << ":" << port
       << ") and key " << pub_k << " wants to join." << endl;
}
