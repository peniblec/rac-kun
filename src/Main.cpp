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

  // setup an "io_service", ie a Boost.Asio object which will create sockets,
  // call handlers, start timers... on demand
  shared_ptr<asio::io_service> io_service(new asio::io_service);
  shared_ptr<tcp::resolver> resolver(new tcp::resolver(*io_service));

  // create minimal network: the local peer, and an (empty) network view
  shared_ptr<Peer> local_peer = create_local_peer();
  shared_ptr<Network> network(new Network(io_service, resolver, local_peer));

  // the acceptor
  Listener listener(io_service, network);

  // launch io_service.run() in another thread
  thread io_service_thread(bind(&asio::io_service::run, io_service));

  // if an entry point was specified in configuration, try to connect right away
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

      if (local_peer->get_state() != PEER_STATE_NEW)
        cout << "Cannot join while "
             << PeerStateNames[local_peer->get_state()] << endl;

      else {
        int colon = argument.find(':');
        string ip( argument.substr(0, colon) );
        string port( argument.substr(colon+1, argument.size()) );

        network->join(ip, port);
      }
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
         << endl;
    }
    else
      cout << "Invalid command." << endl;
    
  }

  io_service_thread.join();

  return 0;
}
