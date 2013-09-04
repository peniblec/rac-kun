#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

#define MESSAGE_SIZE 128
#define RINGS_NB 3

const string COMMAND_SEND = "send";
const string COMMAND_JOIN = "join";
const string COMMAND_RINGS = "rings";
const string COMMAND_BCAST = "broadcast";
const string COMMAND_LOGS = "logs";

enum PeerState {
  PEER_STATE_NEW = 0,
  PEER_STATE_JOINING,
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


extern const unsigned int MSG_STAMP_LENGTH;
extern const unsigned int MSG_STAMP_OFFSET;

extern const unsigned int DATA_MESSAGE_OFFSET;

extern const unsigned int ID_LENGTH;

extern const unsigned int JOIN_MSG_ID_OFFSET;
extern const unsigned int JOIN_MSG_ID_LENGTH;

extern const unsigned int JOIN_MSG_KEY_OFFSET;
extern const unsigned int JOIN_MSG_KEY_LENGTH;

extern const unsigned int JOIN_NOTIF_ENDPOINT_OFFSET;

extern const unsigned int READY_TIME;
extern const unsigned int JOIN_COMPLETE_TIME;


// configurable settings

class Settings {
public:
  unsigned short LISTEN_PORT;
  string ENTRY_POINT_IP;
  unsigned short ENTRY_POINT_PORT;
  bool UI;
};
  
extern Settings settings;

void init_settings(int argc, char** argv);


#endif 
