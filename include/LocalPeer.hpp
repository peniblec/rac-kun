#ifndef LOCALPEER_H
#define LOCALPEER_H


#include "Config.hpp"


class LocalPeer {
public:
  typedef LocalState State;

  LocalPeer();
  void set_state(State new_state);
  State const get_state();
  void update();
  
private:
  string id;
  string pub_key;
  State state;
};
  
#endif
