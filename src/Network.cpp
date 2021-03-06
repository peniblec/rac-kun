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

#include <boost/bind.hpp>

#include "DataMessage.hpp"
#include "JoinMessage.hpp"
#include "JoinNotifMessage.hpp"
#include "JoinAckMessage.hpp"
#include "ReadyMessage.hpp"
#include "ReadyNotifMessage.hpp"

#include "Network.hpp"

Network::Network(shared_ptr<asio::io_service> _ios,
                 shared_ptr<tcp::resolver> _resolver,
                 shared_ptr<Peer> _local_peer)
  : io_service(_ios), resolver(_resolver),
    local_peer(_local_peer), join_token(true),
    h_logs( logs.get<LOG_INDEX_HASH>() ), t_logs( logs.get<LOG_INDEX_TIME>() )
{
}

void Network::add_new_peer(shared_ptr<Peer> p)
{
  // store info about how to reach this new peer, start listening for incoming
  // messages

  new_peers[ p->get_address() ] = pair<shared_ptr<Peer>, unsigned short>(p, 0);

  Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                      asio::placeholders::error,
                                      asio::placeholders::bytes_transferred, p);
  p->start_listening(listen_handler);
}

void Network::join(string entry_ip, string entry_port)
{
  // create a peer corresponding to the entry point (keep it NEW, if he does not
  // belong to our future group, we should not care about it for now), try to
  // reach him, send a JOIN request

  try {
    shared_ptr<Peer> entry_peer = connect_peer(entry_ip, entry_port);
    local_peer->set_state(PEER_STATE_JOINING);

    add_new_peer(entry_peer);
  
    Message* join = new JoinMessage(local_peer->get_id(), local_peer->get_key(),
                                    settings.LISTEN_PORT);
    send( join, entry_peer );
    delete join;
  }
  catch (std::exception& e) {
    DEBUG("Network::join: " << e.what());
  }
}

shared_ptr<Peer> Network::connect_peer(string ip, string port)
{
  // try to connect to the specified endpoint, create a peer with the resulting
  // socket

  tcp::resolver::query query(ip, port);
  tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

  shared_ptr<tcp::socket> socket(new tcp::socket(*io_service));

  // for Boost versions up to 1.47
  tcp::resolver::iterator end;
  system::error_code error = boost::asio::error::host_not_found;
  while (error && endpoint_iterator != end) {
    socket->close();
    socket->connect(*endpoint_iterator++, error);
  }
  if (error)
    throw system::system_error(error);
  // shorter call for Boost version after 1.48
  // asio::connect(*socket, endpoint_iterator);

  shared_ptr<Peer> new_peer(new Peer(socket));

  return new_peer;
}

void Network::broadcast(shared_ptr<Group> group, BCastMessage* message,
                        bool add_stamp)
{
  // create a unique ID (stamp) for the message if asked, send it to the
  // successors from the correct group, then log the message
  
  if (add_stamp)
    message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());

  PeerMap succs = group->get_successors();

  PeerMap::iterator it;
  for (it=succs.begin(); it!=succs.end(); it++) {
    it->second->send(msg);
  }
  log_message( message, local_peer );
}

void Network::broadcast_data(string content)
{
  // use local group rings to broadcast some data

  DataMessage* data = new DataMessage(content);

  data->BCAST_MARKER = make_channel_marker(local_group, local_group);
  broadcast(local_group, data, true);
  delete data;
}

void Network::send_all(string message)
{
  DEBUG("\tSending cleartext to all peers...");
  PeerMap::iterator it;
  for (it=peers.begin(); it!=peers.end(); it++) {

    it->second->send(message);
  }
}

void Network::send(Message* message, shared_ptr<Peer> peer)
{
  // make stamp, send, log

  message->make_stamp( local_peer->get_id() );
  string msg(message->serialize());

  peer->send(msg);
  log_message(message, local_peer);
}

void Network::send_ready(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    Message* ready = new ReadyMessage();
    send(ready, peer);
    delete ready;

    ready_timers.erase( peer->get_id() );
  }
  else {
    DEBUG("Network::send_ready: " << error.message());
  }
}

void Network::answer_join_request(shared_ptr<Peer> peer, unsigned short port)
{
  // grab join token, determine correct group for this new peer, broadcast a
  // JOIN Notification to this group

  join_token = false;
  shared_ptr<Group> group = local_group;
  // TODO: replace with computation based on peer IDs - see RAC III.C.Joining

  BCastMessage* notif = new JoinNotifMessage( false, group->get_id(),
                                              peer->get_id(), peer->get_key(),
                                              peer->get_address(), port );
  notif->BCAST_MARKER = make_channel_marker( local_group, group );

  broadcast(group, notif, true);
  delete notif;

  new_peers.erase( peer->get_address() );

  // if the peer is sorted into our group, send a JOIN Acknowledgement right
  // away (we won't care about the Notif since we sent it)
  // else, we'll send the Ack when the peer is ready to join channels
  if (group->get_id() == local_group->get_id())
    acknowledge_join(peer, group);

  // set timer before sending READY signal
  shared_ptr<asio::deadline_timer> t
    (new asio::deadline_timer(*io_service, posix_time::seconds(READY_TIME)));
  ready_timers[ peer->get_id() ] = t;

  t->async_wait( bind(&Network::send_ready, this,
                      asio::placeholders::error, peer) );
  
}

void Network::check_for_new_peers()
{
  JoinMap::iterator it = new_peers.begin();

  // ignore nodes who haven't sent a JOIN request yet
  while ( it!=new_peers.end() && !(it->second.first->is_known()) )
    it++;

  if (it!=new_peers.end())
    answer_join_request( it->second.first, it->second.second );
  else
    join_token = true;

}

void Network::complete_join(const system::error_code& error, shared_ptr<Peer> peer)
{
  if (!error) {
    peer->set_state(PEER_STATE_CONNECTED);
    join_timers.erase( peer->get_id() );
    
    check_for_new_peers();

  }
  else {
    DEBUG("Network::complete_join: " << error.message());
  }
}

void Network::handle_disconnect(shared_ptr<Peer> p)
{
   DEBUG("Peer @" << p->get_address() << " disconnected.");
    if ( peers.erase( p->get_id() ) ) {

      if (local_group->remove_peer(p)) {
        // the peer belonged to our group, so we'll have to update all our
        // channels 
        GroupMap::iterator it;
        for (it = groups.begin(); it!=groups.end(); it++) {
          it->second->remove_peer(p);
          it->second->update_neighbours(local_peer);
        }
      }
      else {
        // the peer belonged to another group, so we'll just have to update that
        // one 
        bool found(false);
        GroupMap::iterator it = groups.begin();

        while (!found && it!=groups.end()) {

          if ( it->second->remove_peer(p) ) {
            it->second->update_neighbours(local_peer);
            found = true;
          }
        }
      }
    }
    else {
      new_peers.erase( p->get_address() );
    }
    if (peers.size()==0 && new_peers.size()==0) {
      groups.clear(); // TODO: last-week-of-internship effort to clean stuff,
                      // maybe not actually useful
      local_group.reset();
      local_peer->set_state(PEER_STATE_NEW);
    }
}

void Network::handle_incoming_message(const system::error_code& error,
                                      size_t bytes_transferred,
                                      shared_ptr<Peer> emitter)
{
  // EOF is the error raised by Boost.Asio when the transmission is closed
  if (error == asio::error::eof) 
    handle_disconnect(emitter);

  else if (error) {
    DEBUG("Network::handle_incoming_message: " << error.message());
  }

  else {
    string received_message = emitter->get_last_message(bytes_transferred);
    emitter->listen();

    try {
      Message* message = parse_message(received_message);

      LogIndexHash::iterator it = find_log(message);

      if (it != h_logs.end()) {
        // either we sent this message, or we've already received it
        // (and forwarded it), so we shouldn't have anything else to do

        h_logs.modify(it, ack_message(emitter));
        delete message;

        return;

      }
      else  {
        if (emitter->is_known())
          log_message(message, emitter);

        if (message->is_broadcast()) {
          // use the broadcast marker to figure out in which rings it belongs
          // (either our local group's rings, or some channel's rings)
          BCastMessage* bcast_message = dynamic_cast<BCastMessage*>(message);
          string bcast_marker = bcast_message->BCAST_MARKER;

          if (channel_markers.count(bcast_marker))
            broadcast(groups[ channel_markers[bcast_marker] ], bcast_message);

        }
      }
      // process the message
      switch (message->type) {

      case MESSAGE_TYPE_JOIN: {
        if (local_peer->get_state() != PEER_STATE_CONNECTED) {
          // if we've been alone so far, create a group and pose as a functional
          // network 
          local_peer->set_state(PEER_STATE_CONNECTED);

          string concat( local_peer->get_id()
                         + itos(milliseconds_since_epoch()) ) ;
          string id = make_hash(concat);
          local_group = create_group(id);

          channel_markers.insert
            ( pair<string, string>
              (make_channel_marker(local_group, local_group), id) );

          local_group->add_to_rings(local_peer);
        }

        // fill info about the peer, log his request
        
        JoinMessage* msg = dynamic_cast<JoinMessage*>(message);

        emitter->init( msg->id, msg->pub_k );
        emitter->set_state( PEER_STATE_JOINING );

        log_message(message, emitter);

        // as the entry point for the emitter, if no other peer is currently in
        // the process of joining, we will answer his request
        // else, we'll store his contact information and call him later

        if (join_token)
          answer_join_request(emitter, msg->port);
        else
          new_peers[ emitter->get_address() ].second = msg->port;
      }
        break;

      case MESSAGE_TYPE_JOIN_NOTIF: {
        
        JoinNotifMessage* msg = dynamic_cast<JoinNotifMessage*>(message);

        GroupMap::iterator it = groups.find( msg->group_id );

        // if the group exists, and ( it's ours ) ^ ( the peer is joining
        // channels now ), acknowledge his join

        if ( it!=groups.end() && ( (msg->group_id==local_group->get_id())
                                   ^ msg->CHANNEL ) ) { 
          // TODO: check that ID belongs to that group (cf RAC III.C.Joining)

          join_token = false;
          shared_ptr<Peer> new_peer = connect_peer( msg->ip,
                                                    itos(msg->port) );

          new_peer->init( msg->peer_id, msg->pub_k );
          new_peer->set_state(PEER_STATE_JOINING);

          Peer::Handler listen_handler = bind(&Network::handle_incoming_message, this,
                                              asio::placeholders::error,
                                              asio::placeholders::bytes_transferred,
                                              new_peer);
          new_peer->start_listening(listen_handler);

          acknowledge_join(new_peer, it->second);
        }

        // if it's our group and the channel flag is on, that means we should
        // already know him, so we just have to add him to channel rings

        else if ( it!=groups.end() && msg->CHANNEL ) { // implying group==local
          PeerMap::iterator p_it = peers.find( msg->peer_id );

          if ( p_it != peers.end() ) {
            shared_ptr<Peer> peer = p_it->second;

            // update the channels
            for (GroupMap::iterator it=groups.begin(); it!=groups.end(); it++) {

              if (it->second->get_id() != local_group->get_id()) {
                it->second->add_to_rings(peer);
                it->second->update_neighbours(local_peer);
              }
            }

          }

        }
      }
        break;

      case MESSAGE_TYPE_JOIN_ACK: {

        // we're joining, group members start making themselves known
        // store info about this peer, sort him into the group he says

        JoinAckMessage* msg = dynamic_cast<JoinAckMessage*>(message);
        
        shared_ptr<Group> his_group;
        GroupMap::iterator it = groups.find( msg->group_id );
        if (it == groups.end()) {
          // we don't know this group yet, so let's create it and add all the
          // members from our groups to the channel rings (don't bother with
          // update_neighbours, we'll call it when we receive the READY signal)

          his_group = create_group( msg->group_id );

          if (!local_group)
            local_group = his_group;
          else
            his_group->add_to_rings(local_group);

          channel_markers.insert(pair<string, string>
                                 ( make_channel_marker(local_group, his_group),
                                   msg->group_id ));
        }
        else
          his_group = it->second;

        emitter->init( msg->peer_id, msg->pub_k );
        emitter->set_state(PEER_STATE_CONNECTED);

        new_peers.erase( emitter->get_address() );
        peers[ emitter->get_id() ] = emitter;

        his_group->add_peer(emitter);

        log_message( message, emitter );
        
      }
        break;

      case MESSAGE_TYPE_READY: {

        // members from our group (if we're JOINING) or from all groups (if
        // we're CONNECTED) have all added us to their view and sent us JOIN
        // Acknowledgements, so now we can compute our position on the rings

        for (GroupMap::iterator it = groups.begin(); it!=groups.end(); it++) {

          shared_ptr<Group> group = it->second;

          bool notify = ( group->get_id() != local_group->get_id()
                          || local_peer->get_state() != PEER_STATE_CONNECTED );
          // ie only send Notifs to local group if we're not CONNECTED

          // if this is not the local group, just add ourselves to the channel
          // rings 
          if (group->get_id() != local_group->get_id()) {
            group->add_to_rings(local_peer);
            group->update_neighbours(local_peer);
          }
          // else if this is the local group and we're still JOINING, add
          // ourself to the group rings
          else if (local_peer->get_state() != PEER_STATE_CONNECTED) {
            local_peer->set_state(PEER_STATE_CONNECTED);
            local_group->add_to_rings(local_peer);
            local_group->update_neighbours(local_peer);
          }

          if ( notify ) {
          // send READY Notification to our direct predecessors and successors
            Message* notif = new ReadyNotifMessage();

            notif->make_stamp(local_peer->get_id());
            string notif_msg = notif->serialize();

            PeerMap preds = group->get_predecessors();
            PeerMap succs = group->get_successors();

            PeerMap directs(preds);
            directs.insert( succs.begin(), succs.end() );

            for (PeerMap::iterator it=directs.begin(); it!=directs.end(); it++)
              it->second->send(notif_msg);

            log_message( notif, local_peer );
        
            delete notif;
          }
        }
      }
        break;

      case MESSAGE_TYPE_READY_NOTIF: {

        // emitter is now ready to be used as relay
        // his JOIN procedure his finished, let's see whether some other peers
        // were trying to join

        emitter->set_state(PEER_STATE_CONNECTED);
        check_for_new_peers();
      }
        break;

      case MESSAGE_TYPE_DATA: {

        DataMessage* msg = dynamic_cast<DataMessage*>(message);
        
        cout << "Peer " << ( emitter->is_known() ? emitter->get_id()
                             : string("@" + emitter->get_address()) )
             << " sent this:\n\t" << msg->data << endl;
      }
        break;
        
      default: 
        throw MessageParseException();
      }

      delete message;
    }
    catch (std::exception& e) {
      cout << e.what() << endl;
      cout << "Message bytes:" << endl;

      display_chars(received_message, received_message.size());
      cout << endl;
    }
  }  
}

void Network::acknowledge_join(shared_ptr<Peer> peer, shared_ptr<Group> group)
{
  // add peer to the group, update our neighbours in the relevant rings

  peers[ peer->get_id() ] = peer;

  group->add_peer(peer);
  group->update_neighbours(local_peer);

  // send a JOIN Acknowledgement so that he knows about us

  Message* ack = new JoinAckMessage( local_group->get_id(),
                                     local_peer->get_id(),
                                     local_peer->get_key() );
  send(ack, peer);
  delete ack;

  PeerMap preds = group->get_predecessors();
  PeerMap succs = group->get_successors();

  // if the peer has not become a direct neighbour, start a timer until we
  // consider him ready to act as a relay
  // (else, wait for the READY Notification)
  
  if ( preds.find(peer->get_id()) == preds.end()
       && succs.find(peer->get_id()) == succs.end() ) {

    shared_ptr<asio::deadline_timer> t
      (new asio::deadline_timer(*io_service,
                                posix_time::seconds(JOIN_COMPLETE_TIME)));

    join_timers[ peer->get_id() ] = t;
    t->async_wait( bind(&Network::complete_join, this,
                        asio::placeholders::error, peer) );
  }
}

void Network::print_rings()
{
  // display info about the local peer (since he is not sorted in any group),
  // then go group by group

  cout << "This peer: " << local_peer->get_id() << endl;
  GroupMap::iterator g_it;
  for (g_it = groups.begin(); g_it != groups.end(); g_it++) {

    cout << "\tIn group ";
    display_chars(g_it->first, 10);
    cout << ( g_it->second->get_id() == local_group->get_id() ?
              " (LOCAL): " : ": " )
         << endl;
    
    g_it->second->display_rings();

    PeerMap preds = g_it->second->get_predecessors();
    PeerMap succs = g_it->second->get_successors();

    cout << "My predecessors are:" << endl;
    for (PeerMap::iterator it=preds.begin(); it!=preds.end(); it++) {
      cout << "- " << it->second->get_id() << endl;
    }
    cout << "My successors are:" << endl;
    for (PeerMap::iterator it=succs.begin(); it!=succs.end(); it++) {
      cout << "- " << it->second->get_id() << endl;
    }

    cout << endl;

  }
}

void Network::print_logs()
{
  // display every message in order of emission/reception, whom we received it
  // from, and how many times

  for (LogIndexTime::iterator it=t_logs.begin(); it!=t_logs.end(); it++) {
    Message* m = parse_message( it->message );
    m->display();
    cout << "Received from:" << endl;

    map<string, int> preds = it->control;
    for (map<string, int>::iterator jt=preds.begin(); jt!=preds.end(); jt++) {
      cout << "- " << jt->first << ": " << jt->second << endl;
    }
    cout << endl;
    delete m;
  }
}

void Network::log_message(Message* message, shared_ptr<Peer> emitter)
{
  LogIndexHash::iterator it = find_log(message);
  
  if ( it==h_logs.end() ) {
    // if this is the first time we receives the message, build the structure

    MessageLog ml;
    ml.message = message->serialize();

    // make a list of peers we expect to receive this message from, using
    // BCAST_MARKER to find in which rings it will be broadcast

    if (message->is_broadcast()) {
      BCastMessage* bmsg = dynamic_cast<BCastMessage*>(message);
      string bcast_marker = bmsg->BCAST_MARKER;

      PeerMap preds
        = groups[ channel_markers[bcast_marker] ]->get_predecessors();
      
      for (PeerMap::iterator p=preds.begin(); p!=preds.end(); p++)
        ml.control[ p->second->get_id() ] = 0;
    }
    pair<LogIndexHash::iterator, bool> pair = h_logs.insert( ml );
    if (pair.second) // insertion shouldn't fail because of a similar element
      it = pair.first;
  }
            
  if ( !emitter->is_local() && it!=h_logs.end() ) // only acknowledge reception
                                                  // if we're not the emitter
                                                  // (and iterator is valid)
    h_logs.modify(it, ack_message(emitter));
}

Network::LogIndexHash::iterator Network::find_log(Message* message) {
  // build minimal MessageLog struct to find the corresponding message
  MessageLog ml;
  ml.message = message->serialize();

  return h_logs.find(ml);
}

shared_ptr<Group> Network::create_group(string id)
{
  // create group, add it to the global group list

  shared_ptr<Group> new_group
    = shared_ptr<Group>(new Group(id));

  groups.insert( pair<string, shared_ptr<Group> >(id, new_group) );

  return new_group;
}

string Network::make_channel_marker(shared_ptr<Group> group1,
                                    shared_ptr<Group> group2)
{
  // put "lowest" ID first as a convention
  return ( group1->get_id() < group2->get_id()
           ? make_hash(group1->get_id() + group2->get_id())
           : make_hash(group2->get_id() + group1->get_id()) );
}
