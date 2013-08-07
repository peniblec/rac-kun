#include "Config.hpp"

const char* PeerStateNames[PEER_STATE_END] =
  {
    "Connecting",
    "Alive",
    "Dead"
  };

const char* MessageTypeNames[MESSAGE_TYPE__END] =
  {
    "None",
    "Join Request",
    "Join Notification",
    "Ready Signal",
    "Ready Notification",
    "Data"
  };
