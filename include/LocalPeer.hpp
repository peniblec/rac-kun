#ifndef LOCALPEER_H
#define LOCALPEER_H


#include "Config.hpp"


class LocalPeer {
public:
  typedef LocalState State;

  LocalPeer();
  void set_state(State new_state);
  State const get_state();
  string const get_id();
  string const get_pub_key();
  
private:
  string id;
  string pub_key;
  State state;
};
  
#endif
