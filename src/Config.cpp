#include "Config.hpp"

const char* PeerStateNames[PEER_STATE_END] =
  {
    "Connecting",
    "Alive",
    "Dead"
  };

const char* LocalStateNames[LOCAL_STATE_END] =
  {
    "Alone",
    "Joining",
    "Connected"
  };

const char* MessageTypeNames[MESSAGE_TYPE_END] =
  {
    "None",
    "Join Request",
    "Join Notification",
    "Ready Signal",
    "Ready Notification",
    "Data"
  };
