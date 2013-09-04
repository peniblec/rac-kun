#include <boost/program_options.hpp>
#include <cryptopp/sha.h>

#include "Config.hpp"

const char* PeerStateNames[PEER_STATE_END] =
  {
    "New",
    "Joining",
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

const unsigned int MSG_STAMP_LENGTH = CryptoPP::SHA1::DIGESTSIZE;
const unsigned int MSG_STAMP_OFFSET = 1;

const unsigned int ID_LENGTH = 13;

const unsigned int DATA_MESSAGE_OFFSET = MSG_STAMP_OFFSET + MSG_STAMP_LENGTH;

const unsigned int JOIN_MSG_ID_OFFSET = MSG_STAMP_OFFSET + MSG_STAMP_LENGTH;
const unsigned int JOIN_MSG_ID_LENGTH = ID_LENGTH;

const unsigned int JOIN_MSG_KEY_OFFSET = JOIN_MSG_ID_OFFSET + JOIN_MSG_ID_LENGTH;
const unsigned int JOIN_MSG_KEY_LENGTH = 5;

const unsigned int JOIN_NOTIF_ENDPOINT_OFFSET = JOIN_MSG_KEY_OFFSET + JOIN_MSG_KEY_LENGTH;

const unsigned int READY_TIME = 3; // seconds
const unsigned int JOIN_COMPLETE_TIME = 2*READY_TIME;


void init_settings(int argc, char** argv)
{
  using namespace boost::program_options;

  options_description config("Configuration");
  config.add_options()
    ("listen_port,l", value<unsigned short>(),
     "The port on which incoming connections will be handled (default: random)")
    ("entry_point,e", value<string>(),
     "The pair \"ip/hostname:port\" for the entry point to connect to (default: none, program will wait for a JOIN command)")
    ("ui", value<bool>(),
     "Show user interface, enable commands (default: true)")
    ("config_file", value<string>(),
     "The location for the configuration file (default: \"rac.conf\")")
    ("help,h",
     "Displays options specification, stops program")
    ;
  positional_options_description pos_options;
  pos_options.add("config_file", 1);

  variables_map cmd_line_map, conf_file_map;

  store(command_line_parser(argc, argv).options(config).positional(pos_options).run(),
        cmd_line_map);
  notify(cmd_line_map);

  if (cmd_line_map.count("help")) {
    cout << config << endl;
    exit(EXIT_SUCCESS);
  }

}
