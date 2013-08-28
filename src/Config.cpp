#include <cryptopp/sha.h>

#include "Config.hpp"

const char* PeerStateNames[PEER_STATE_END] =
  {
    "New",
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

extern const unsigned int MSG_STAMP_LENGTH = CryptoPP::SHA1::DIGESTSIZE;
extern const unsigned int MSG_STAMP_OFFSET = 1;

extern const unsigned int ID_LENGTH = 13;

extern const unsigned int DATA_MESSAGE_OFFSET = MSG_STAMP_OFFSET + MSG_STAMP_LENGTH;

extern const unsigned int JOIN_MSG_ID_OFFSET = MSG_STAMP_OFFSET + MSG_STAMP_LENGTH;
extern const unsigned int JOIN_MSG_ID_LENGTH = ID_LENGTH;

extern const unsigned int JOIN_MSG_KEY_OFFSET = JOIN_MSG_ID_OFFSET + JOIN_MSG_ID_LENGTH;
extern const unsigned int JOIN_MSG_KEY_LENGTH = 5;

extern const unsigned int JOIN_NOTIF_IP_OFFSET = JOIN_MSG_KEY_OFFSET + JOIN_MSG_KEY_LENGTH;

extern const unsigned int READY_TIME = 3; // seconds
extern const unsigned int JOIN_COMPLETE_TIME = 2*READY_TIME;
