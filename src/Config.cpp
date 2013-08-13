#include "Config.hpp"

const char* PeerStateNames[PEER_STATE_END] =
  {
    "New",
    "Joining",
    "Readying",
    "Connected"
  };

const char* LocalStateNames[LOCAL_STATE_END] =
  {
    "Alone",
    "Joining",
    "Readying",
    "Connected"
  };

const char* MessageTypeNames[MESSAGE_TYPE_END] =
  {
    "None",
    "Join Request",
    "Join Notification",
    "Join Acknowledgement",
    "Ready Signal",
    "Ready Notification",
    "Data"
  };
