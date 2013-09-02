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


int main() {

  // setup and start local_listener in other thread
  shared_ptr<asio::io_service> io_service(new asio::io_service);
  shared_ptr<tcp::resolver> resolver(new tcp::resolver(*io_service));

  shared_ptr<Peer> local_peer = create_local_peer();
  shared_ptr<Network> network(new Network(io_service, resolver, local_peer));

  Listener listener(io_service, network);

  thread io_service_thread(bind(&asio::io_service::run, io_service));

  cout << "What to do?" << endl
       << "send <message> - Send <message> to all peers" << endl
       << "join <address> - Join a session using <address> as an entry point" << endl
       << "broadcast <message> - Use rings to broadcast a message" << endl
       << "rings - Display the constitution of the current rings" << endl
       << "logs - Print all the messages received up until now" << endl;

  for(;;) {

    string input, command, argument;
    getline(cin, input);
    parse_input(input, command, argument);


    if ( command.compare(COMMAND_SEND)==0 ) {

      network->send_all(argument);
    }
    else if ( command.compare(COMMAND_JOIN)==0 ) {

      network->join(argument, itos(RAC_PORT));
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
    
  }

  return 0;
}
