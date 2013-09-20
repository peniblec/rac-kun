/*

  RAC-kun - junior version of RAC, a freerider-resilient, scalable, anonymous
  communication protocol conceived by researchers from CNRS and LIG.
  Copyright (C) 2013  Kévin Le Gouguec

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see [http://www.gnu.org/licenses/].

*/
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
const unsigned int BCAST_MARKER_LENGTH = CryptoPP::SHA1::DIGESTSIZE;

const unsigned int ID_LENGTH = 20;
const unsigned int GROUP_ID_LENGTH = CryptoPP::SHA1::DIGESTSIZE;

const unsigned int KEY_LENGTH = 5;


const unsigned int READY_TIME = 3; // seconds
const unsigned int JOIN_COMPLETE_TIME = 2*READY_TIME;


Settings settings;

void init_settings(int argc, char** argv)
{
  using namespace boost::program_options;

  // specify command line and configuration file options

  options_description config("Configuration");
  config.add_options()
    ("listen_port,l", value<unsigned short>(),
     "The port on which incoming connections will be handled"
     "\n- example: any number in [1, 65535]"
     "\n- default: random\n")
    ("entry_point,e", value<string>(),
     "The pair \"ip/hostname:port\" specifying the entry point to connect to"
     "\n- example: 127.0.0.1:1764"
     "\n- default: none, will wait for a JOIN command\n")
    ("ui", value<bool>(),
     "Show user interface, enable commands"
     "\n- example: true, false"
     "\n- default: true\n")
    ("config_file", value<string>(),
     "The location for the configuration file"
     "\n(\"--config_file\" may be omitted)"
     "\n- default: \"rac.conf\"\n")
    ("help,h",
     "Displays options specification, stops program")
    ;

  // allow config file to be specified without "--config_file"
  positional_options_description pos_options;
  pos_options.add("config_file", 1);

  variables_map cmd_line_map;

  try {
    store(command_line_parser(argc, argv)
          .options(config).positional(pos_options).run(), cmd_line_map);
    notify(cmd_line_map);
  }
  catch (std::exception& e) {
    cout << "Failed to parse command arguments:" << endl
         << "\t" << e.what() << endl;
    cmd_line_map.clear();
  }
  
  if (cmd_line_map.count("help")) {
    // show configuration and commands, then exit

    cout << config << endl << endl;
    cout << "UI commands:" << endl
         << "\tsend <arg>" << endl
         << "\t\tSend <arg> to all peers" << endl
         << "\tjoin <arg>" << endl
         << "\t\tJoin a session using <arg> as an entry point" << endl
         << "\t\twhere <arg> is IP:port or hostname:port" << endl
         << "\tbroadcast <arg>" << endl
         << "\t\tUse rings to broadcast a message" << endl
         << "\trings" << endl
         << "\t\tDisplay the constitution of the current rings" << endl
         << "\tlogs" << endl
         << "\t\tPrint all the messages received up until now" << endl
         << "\thelp" << endl
         << "\t\tDisplay all available commands" << endl
         << "\tquit" << endl
         << "\t\tExit application" << endl
         << endl;

    exit(EXIT_SUCCESS);
  }

  // find configuration file in command line, or try to open "rac.conf"
  string config_file = ( cmd_line_map.count("config_file") ?
                         cmd_line_map["config_file"].as<string>() :
                         "rac.conf" );
  
  variables_map conf_file_map;
  try {
    store(parse_config_file<char>(config_file.c_str(), config), conf_file_map);
    notify(conf_file_map);
  }
  catch (std::exception& e) {
    cout << "Failed to parse configuration file:" << endl
         << "\t" << e.what() << endl;
    conf_file_map.clear();
  }

  // find listen port, default to random
  try {
    settings.LISTEN_PORT =
      ( cmd_line_map.count("listen_port")
        ? cmd_line_map["listen_port"].as<unsigned short>()
        : ( conf_file_map.count("listen_port")
            ? conf_file_map["listen_port"].as<unsigned short>()
            : 0 ) );
  }
  catch (std::exception& e) {
    cout << "Failed to interpret port from configuration:" << endl
         << "\t" << e.what() << endl << "\t(defaulting to random)" << endl;
  }

  
  // find entry point
  // if we can't parse the remote port, set it to 0 to tell the app not to try
  // to connect
  if (cmd_line_map.count("entry_point") || conf_file_map.count("entry_point")) {

    string endpoint = ( cmd_line_map.count("entry_point")
                        ? cmd_line_map["entry_point"].as<string>()
                        : conf_file_map["entry_point"].as<string>() );

    int colon = endpoint.find(':');

    string ip( endpoint.substr(0, colon) );
    settings.ENTRY_POINT_IP = ip;
    
    unsigned short port;
    if ( (istringstream( endpoint.substr(colon+1, endpoint.size()) ) >> port)
         .fail() ) {
      settings.ENTRY_POINT_PORT = 0;
      cout << "Failed to interpret remote entry point's listening port" << endl
           << "\t(defaulting to no entry point: use join command)" << endl;
    }
    else
      settings.ENTRY_POINT_PORT = port;
                        
  }
  else
    settings.ENTRY_POINT_PORT = 0;

  // find whether user wants UI or not, default to true
  try {
    settings.UI = ( cmd_line_map.count("ui")
                    ? cmd_line_map["ui"].as<bool>()
                    : ( conf_file_map.count("ui")
                        ? conf_file_map["ui"].as<bool>()
                        : true ) );
  }
  catch (std::exception& e) {

    cout << "Failed to interpret UI from configuration:" << endl
         << "\t" << e.what() << endl << "\t" << "(defaulting to true)" << endl;
  }
}
