#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>

#include "Config.hpp"
#include "Listener.hpp"
#include "Network.hpp"
#include "Peer.hpp"
#include "Utils.hpp"

using namespace boost;
using boost::asio::ip::tcp;


int main(int argc, char** argv) {

  init_settings(argc, argv);

  // setup and start local_listener in other thread
  shared_ptr<asio::io_service> io_service(new asio::io_service);
  shared_ptr<tcp::resolver> resolver(new tcp::resolver(*io_service));

  shared_ptr<Peer> local_peer = create_local_peer();
  shared_ptr<Network> network(new Network(io_service, resolver, local_peer));

  Listener listener(io_service, network);

  thread io_service_thread(bind(&asio::io_service::run, io_service));

  if ( settings.ENTRY_POINT_PORT )
    network->join(settings.ENTRY_POINT_IP, itos(settings.ENTRY_POINT_PORT));


  while (settings.UI) {

    string input, command, argument;
    getline(cin, input);
    parse_input(input, command, argument);


    if ( command.compare(COMMAND_SEND)==0 ) {

      network->send_all(argument);
    }
    else if ( command.compare(COMMAND_JOIN)==0 ) {

      int colon = argument.find(':');
      string ip( argument.substr(0, colon) );
      string port( argument.substr(colon+1, argument.size()) );

      network->join(ip, port);
    }
    else if ( command.compare(COMMAND_RINGS)==0 ) {

      network->print_rings();
    }
    else if ( command.compare(COMMAND_BCAST)==0 ) {

      network->broadcast_data(argument);
    }
    else if ( command.compare(COMMAND_LOGS)==0 ) {

      network->print_logs();
    }
    else if ( command.compare(COMMAND_HELP)==0 ) {

      cout << "What to do?" << endl
           << "send <message>\tSend <message> to all peers" << endl
           << "join <address>\tJoin a session using <address> as an entry point" << endl
           << "broadcast <message>\tUse rings to broadcast a message" << endl
           << "rings\tDisplay the constitution of the current rings" << endl
           << "logs\tPrint all the messages received up until now" << endl;
    }
    else
      cout << "Invalid command." << endl;
    
  }

  io_service_thread.join();

  return 0;
}
