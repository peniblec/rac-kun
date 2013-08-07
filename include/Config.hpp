#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

#define MESSAGE_SIZE 128
#define RAC_PORT 1337


const string COMMAND_SEND = "send";
const string COMMAND_ADD = "add";

enum PeerState {
  PEER_STATE_CONNECTING = 0,
  PEER_STATE_ALIVE,
  PEER_STATE_DEAD,
  PEER_STATE_END
};

extern const char* PeerStateNames[PEER_STATE_END];

#endif 
