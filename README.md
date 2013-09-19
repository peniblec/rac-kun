RAC-kun - junior version of RAC


Overview
================================================================================

This is a prototype for RAC, a freerider-resilient, scalable, anonymous
communication protocol conceived by researchers from CNRS and LIG.

Implemented features:
- Join procedure
- Ring structure

Implementing:
- Groups and channels

Not implemented:
- ID and pseudonym key pairs
- ID computation
- Onion routing
- Checking for misbehaviour


Requirements
================================================================================

Dependencies:
- Boost 1.49
- Crypto++ 5.6.1


Development notes
================================================================================

Current feature being implemented: groups and channels

What's done:
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

What's not done:
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
  solution: (coming)
- When the group goes below its minimum size, nothing happens. Possible
  solution: (coming)


Credits
================================================================================

RAC stems from the work of the following people:

- Sonia Ben Mokhtar (CNRS)
- Gautier Berthou (Grenoblec University)
- Amadou Diarra (Grenoble University)
- Vivien Quéma (Grenoble INP)
- Ali Shoker (CNRS)
