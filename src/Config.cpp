#include <boost/program_options.hpp>
#include <cryptopp/sha.h>
#include <exception>

#include "Config.hpp"
#include "Utils.hpp"

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

const unsigned int DATA_MESSAGE_OFFSET = MSG_STAMP_OFFSET + MSG_STAMP_LENGTH;

const unsigned int ID_LENGTH = 13;
const unsigned int GROUP_ID_LENGTH = CryptoPP::SHA1::DIGESTSIZE;

const unsigned int JOIN_MSG_ID_OFFSET = MSG_STAMP_OFFSET + MSG_STAMP_LENGTH;
const unsigned int JOIN_MSG_ID_LENGTH = ID_LENGTH;

const unsigned int JOIN_MSG_KEY_OFFSET = JOIN_MSG_ID_OFFSET + JOIN_MSG_ID_LENGTH;
const unsigned int JOIN_MSG_KEY_LENGTH = 5;

const unsigned int JOIN_REQUEST_PORT_OFFSET = JOIN_MSG_KEY_OFFSET + JOIN_MSG_KEY_LENGTH;

const unsigned int JOIN_NOTIF_GROUP_ID_OFFSET = JOIN_MSG_KEY_OFFSET + JOIN_MSG_KEY_LENGTH;

const unsigned int JOIN_NOTIF_ENDPOINT_OFFSET =
  JOIN_NOTIF_GROUP_ID_OFFSET + GROUP_ID_LENGTH;


const unsigned int READY_TIME = 3; // seconds
const unsigned int JOIN_COMPLETE_TIME = 2*READY_TIME;


Settings settings;

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

  variables_map cmd_line_map;

  store(command_line_parser(argc, argv).options(config).positional(pos_options).run(),
        cmd_line_map);
  notify(cmd_line_map);

  if (cmd_line_map.count("help")) {
    cout << config << endl;
    exit(EXIT_SUCCESS);
  }

  string config_file = ( cmd_line_map.count("config_file") ?
                         cmd_line_map["config_file"].as<string>() :
                         "rac.conf" );
  
  variables_map conf_file_map;
  try {
    store(parse_config_file<char>(config_file.c_str(), config), conf_file_map);
    notify(conf_file_map);
  }
  catch (std::exception& e) {
    cout << "Couldn't find configuration file." << endl;
  }

  settings.LISTEN_PORT =
    ( cmd_line_map.count("listen_port")
      ? cmd_line_map["listen_port"].as<unsigned short>()
      : ( conf_file_map.count("listen_port")
          ? conf_file_map["listen_port"].as<unsigned short>()
          : 0 ) );

  if (cmd_line_map.count("entry_point") || conf_file_map.count("entry_point")) {

    string endpoint = ( cmd_line_map.count("entry_point")
                        ? cmd_line_map["entry_point"].as<string>()
                        : conf_file_map["entry_point"].as<string>() );

    int colon = endpoint.find(':');

    string ip( endpoint.substr(0, colon) );
    settings.ENTRY_POINT_IP = ip;
    
    unsigned short port;
    if ( (istringstream( endpoint.substr(colon+1, endpoint.size()) ) >> port).fail() )
      settings.ENTRY_POINT_PORT = 0;
    else
      settings.ENTRY_POINT_PORT = port;
                        
  }
  else
    settings.ENTRY_POINT_PORT = 0;

  settings.UI = ( cmd_line_map.count("ui")
                  ? cmd_line_map["ui"].as<bool>()
                  : ( conf_file_map.count("ui")
                      ? conf_file_map["ui"].as<bool>()
                      : true ) );
}
