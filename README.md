RAC-kun - junior version of RAC


# Overview 

This is a prototype for RAC, a freerider-resilient, scalable, anonymous
communication protocol conceived by researchers from CNRS and LIG.

In its current state, RAC-kun allows a set of nodes to connect and exchange
messages, sorting the participants in ring-like structures to broadcast the
information. The program also keeps track of those messages.


# Installing & Running

## Installation

Dependencies:
- Boost (1.42+)
    - http://sourceforge.net/projects/boost/
- Crypto++ (5.6.1+)
    - http://www.cryptopp.com/

The program will look for headers in `/usr/include` and for library files in
`/usr/lib`. These paths may be changed in the *makefile* that you will find in
the same folder as this readme.

Other requirements:
- This program was developped on Debian 6.0.7 (squeeze), so it should work on
  other OSes based on the Linux kernel (from version 2.6.32-5-686 onward)
- The program assumes that a direct TCP connection may be established between
  two clients, and does not handle any obstruction (NAT, firewall, relay server,
  proxy...)

Once the aforementioned libraries have been installed, you can run `make` in the
same directory as this readme and the program will be compiled into an
executable file named *node*.

## Usage

### Launching

Before running the program, you may write a configuration file in which you can
specify:
- the port the application will use to listen for incoming connections:
  `listen_port = {anything between 1 and 65535}`
- an entry point the program should try to connect upon launch: `entry_point = 
  {hostname or IP address}:{the entry point's listening port}`
- whether or not to use a user interface: `ui = true` or `false`

By default, the program will listen on a random port among those available at
the time of launch, will not try to connect to a remote endpoint, and will
present a user interface.

To use this configuration file, run the program as such: `./node
--config_file={path to the file}` or simply `./node {path to file}`. By default,
the program will try to look for a file named `rac.conf` in the directory it was
called from.

You may also use command line arguments to override any setting defined in the
configuration file, the syntax being `--{setting}={value}` or `--{setting}
{value}`.

Running the program with `--help` will give you an overview of the various
settings, and will also list the command for the user interface.

### Commands

If no endpoint is specified at launch, the program will wait idly until the user
enters a command, eg `join` followed by an endpoint described as `{hostname or
IP address}:{port}`.

When a connection has been successfully established (either by joining, or by
having a remote peer joining us), messages can be exchanged with the `broadcast`
command, followed by the data you want to send. Every peer will receive your
message, and keep track of who in the structure was in charge to forward it to
them.

The messages you have sent and received can be consulted with the `logs`
command, which shows (in order of reception) for each message exchanged
(including protocol messages) whom you received it from (a blank indicates that
this was a one-time signal emitted by the local node).

Note that for each message that you sent with `broadcast`, you receive a copy
from a subset of the other peers: this is because of the protocol's ring
structure, which in addition to allowing messages to be diffused among all
members, lets each node check whether its predecessors did their job and
forwarded the message.

To see this structure, you can use the `rings` command to see, for each group
(see Development Notes for more on the concept of groups), the different rings
composing it, and a recap of your predecessors and successors.

Some other commands:
- `send` (followed by some text) sends the text to all peers at once, bypassing
  the rings.
- `help` will display the list of available commands
- `quit` will exit the application (if `ui` has been set to false, the
  application can still be shut down by sending SIGINT (generally by pressing
  the `ctrl` `C` keys), or other such signals)


# Development Notes

##Features Checklist

Implemented features:
- Join procedure
- Ring structure
- Logs

Implementing:
- Groups and channels

Not implemented:
- ID and pseudonym key pairs
- ID computation
- Onion routing
- Checking for misbehaviour
- A real CLI

## Current Progress

Currently being implemented: groups and channels

### What's done:
- The notions of group and channel exist
- *Network* has a list of groups sorted by ID
- JOIN Notification (resp. Acknowledgements) features a *group_id* field, which
  corresponds to the group the new node should be sorted into (resp. the node
  the acknowledger belongs to)
- Broadcast messages have a channel marker which indicates in which rings they
  are circulating
- Likewise, *Network* has a map associating a channel marker with the
  corresponding group ID
- JOIN Notification and Acknowledgements are ( *should be* ) interpreted
  correctly whether the new node is currently joining its group or the
  surrounding channels
    - NB: *JoinNotification* has a *CHANNEL* flag which indicates whether the
      node is only joining its own group, or the other channels.
      Consider a scenario where the entry point *e*, belonging to group
      G<sub>e</sub>, sorts the new node *n* into group G<sub>n</sub>: *e* will
      have to use the < G<sub>e</sub> - G<sub>n</sub> > channel to send the JOIN
      Notif to G<sub>n</sub>, and we don't want members of G<sub>e</sub> to send
      JOIN Acknowledgements yet; hence the *CHANNEL* flag

### What's not done:
- When the new node has finished joining its group, it does not initiate any
  procedure to join the surrounding channels (and discover the other groups out
  there). Possible solution: (2, 3 and 5 are already implemented)
    1.  either the new node *n*, the entry point, or someone from the new node's
        group starts broadcasting a new JOIN Notif (with the *CHANNEL* flag on)
    2.  all nodes not in *n*'s group will add *n* to their view of its group,
        and update their channel rings ; they will also send JOIN Acks to *n*
    3.  all nodes in *n*'s group will add *n* to all their channels, and update
        the rings 
    4.  the guy from 1. will send READY to *n*
    5.  *n* will only send READY Notifs to members of the other groups

- When the group goes beyond its maximum size, nothing happens. Possible 
  solution:
    1.  after a JOIN procedure is completed, *some node* checks whether the
        group size is too high (if the check is made before the procedure ends,
        only the entry point knows that the group size will get too high, so
        other nodes have no way to check the entry point's claim that a new node
        is joining
    2.  this node broadcasts SPLIT(old group ID) in all channels
    3.  every node computes the ID for the two new groups
        - nodes in the old group compute their new group and channel rings
        - nodes in other groups compute their new channel rings

- When the group goes below its minimum size, nothing happens. Possible
  solution:
    1.  when a member quits, *some node* checks whether the size of group G went
        below the minimal size
    2.  this node broadcasts MERGE(G<sub>old</sub>) in all channels
    3.  for every node *n* in any group G<sub>n</sub>
        - compute where every node of G<sub>old</sub> is dispatched (build a
          list L<sub>G</sub> of the "foster groups" they're joining)
        - if *n* belongs to G<sub>old</sub>,
            - compute rings in his new group
        - else if *n* belongs to a foster group,
            - compute the new group rings in G<sub>n</sub>
        - compute new channel rings between G<sub>n</sub> and any group in
          L<sub>G</sub>


# Credits

RAC stems from the work of the following people:

- Sonia Ben Mokhtar (CNRS)
- Gautier Berthou (Grenoblec University)
- Amadou Diarra (Grenoble University)
- Vivien Quéma (Grenoble INP)
- Ali Shoker (CNRS)

## Implementation

This skeletal implementation was undertaken by a ragtag band of barely-competent
developpers composed of me, myself and I, during my internship with team DRIM in
the LIRIS research lab from CNRS. All complaints about:
- memory leaks
- uncaught exceptions
- awful inheritance patterns
- overall ineptness

may be filed at kevin.lg@free.fr.
