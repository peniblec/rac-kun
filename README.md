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
- Identifier computation
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
- The notion of group and channel exists
- `Network` has a list of groups sorted by ID
- JOIN Notification (resp. Acknowledgements) feature a *group_id* field, which
  corresponds to the group the new node should be sorted into (resp. the node
  the acknowledger belongs to)

What's not done:



Credits
================================================================================

RAC stems from the work of the following people:

- Sonia Ben Mokhtar (CNRS)
- Gautier Berthou (Grenoblec University)
- Amadou Diarra (Grenoble University)
- Vivien Quéma (Grenoble INP)
- Ali Shoker (CNRS)
