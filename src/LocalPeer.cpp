#include <unistd.h>


#include "LocalPeer.hpp"
#include "Utils.hpp"


LocalPeer::LocalPeer()
  : pub_key("AAAAA"), state(LOCAL_STATE_ALONE)
{
  char name[ID_LENGTH + 1];
  name[ID_LENGTH] = '\0';
  // gethostname is not guaranteed to add \0 if it needs to truncate host name

  gethostname(name, sizeof(name) -1);

  id = string(name);
}

void LocalPeer::set_state(LocalPeer::State new_state)
{
  state = new_state;
  DEBUG("Local node now has state " << LocalStateNames[state] << ".");
}
