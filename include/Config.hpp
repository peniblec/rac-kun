#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

#define MESSAGE_SIZE 128
#define RAC_PORT 1337
#define RINGS_NB 3

const string COMMAND_SEND = "send";
const string COMMAND_ADD = "add";
const string COMMAND_JOIN = "join";

enum PeerState {
  PEER_STATE_NEW = 0,
  PEER_STATE_JOINING,
  PEER_STATE_READYING,
  PEER_STATE_CONNECTED,
  PEER_STATE_END
};

extern const char* PeerStateNames[PEER_STATE_END];

enum MessageType {
  MESSAGE_TYPE_NONE = 0,
  MESSAGE_TYPE_JOIN,
  MESSAGE_TYPE_JOIN_NOTIF,
  MESSAGE_TYPE_JOIN_ACK,
  MESSAGE_TYPE_READY,
  MESSAGE_TYPE_READY_NOTIF,
  MESSAGE_TYPE_DATA,
  MESSAGE_TYPE_END
};

extern const char* MessageTypeNames[MESSAGE_TYPE_END];

#define ID_LENGTH (13)

#define JOIN_MSG_ID_OFFSET (1)
#define JOIN_MSG_ID_LENGTH (ID_LENGTH)

#define JOIN_MSG_KEY_OFFSET (JOIN_MSG_ID_OFFSET + JOIN_MSG_ID_LENGTH)
#define JOIN_MSG_KEY_LENGTH (5)

#define JOIN_NOTIF_IP_OFFSET (JOIN_MSG_KEY_OFFSET + JOIN_MSG_KEY_LENGTH)

#define READY_TIME (5) // seconds

#endif 
