#ifndef BCAST_MESSAGE_H
#define BCAST_MESSAGE_H

#include "Message.hpp"


struct BCastMessage : Message // a message passed down group or channel rings
{
  /* is_broadcast:
     - return true
   */
  bool is_broadcast();

  string BCAST_MARKER; // an identifier used to tell the recipient in which
                       // rings (group or channel) this message should be passed
                       // down

protected:
  BCastMessage(Message::Type _type);
};

#endif
