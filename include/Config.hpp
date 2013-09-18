#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using namespace std;

#define MESSAGE_SIZE 1024 // the maximum buffer size used for socket
                          // communication 
#define RINGS_NB 5 // the number of rings per group/channel
#define GROUP_MAX_SIZE 5 // the maximum number of peers per group (UNUSED)

// command names for the CLI
const string COMMAND_SEND = "send";
const string COMMAND_JOIN = "join";
const string COMMAND_RINGS = "rings";
const string COMMAND_BCAST = "broadcast";
const string COMMAND_LOGS = "logs";
const string COMMAND_HELP = "help";

// peer states, represents the various steps of connection
enum PeerState {
  PEER_STATE_NEW = 0, // Aka "unknown", TCP connection established, but no know
                      // ID or key.
  PEER_STATE_JOINING, // Sent a JOIN request, has not finished the JOIN
                      // procedure inside the group yet.
  PEER_STATE_CONNECTED, // Completed the JOIN procedure (may not have joined all
                        // the channels yet).
  PEER_STATE_END
};
extern const char* PeerStateNames[PEER_STATE_END];

// different message types (see individual classes)
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

extern const unsigned int MSG_STAMP_LENGTH; // size of a unique message ID
extern const unsigned int BCAST_MARKER_LENGTH; // size of a unique channel ID

extern const unsigned int ID_LENGTH; // size of a unique peer ID
extern const unsigned int GROUP_ID_LENGTH; // size of a unique group ID

extern const unsigned int KEY_LENGTH; // size of a public ID key


extern const unsigned int READY_TIME; // Amount of time an entry point must wait
                                      // before sending the READY signal to new
                                      // node.
extern const unsigned int JOIN_COMPLETE_TIME; // Amount of time a peer who is
                                              // not a direct pred/succ must
                                              // wait before using the new node
                                              // as relay.

// Configurable settings (see ./program --help
struct Settings {
  unsigned short LISTEN_PORT;
  string ENTRY_POINT_IP;
  unsigned short ENTRY_POINT_PORT;
  bool UI;
};
extern Settings settings;

/* init_settings:
   - called at startup
   - parses command line and configuration file (if found) for settings.
     Settings from command line override those found in the configuratin file.
 */
void init_settings(int argc, char** argv);


#endif 
