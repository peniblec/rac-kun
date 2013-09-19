#ifndef MESSAGE_H
#define MESSAGE_H

#include <exception>

#include "Config.hpp"

struct MessageParseException : public std::exception {
  const char* what() const throw()
  { return "Could not parse message"; }
};

struct Message // a message structured according to the RAC protocol
{
  typedef MessageType Type;

  virtual ~Message();

  /* serialize:
     - get a string containing the message's information, ready to be sent
       across the network
   */
  virtual string serialize();

  /* display:
     - show the message's content
   */
  virtual void display();

  /* make_stamp:
     - create a unique stamp to allow peers to recognize this message when it's
       broadcast

     - peer_id: the original emitter of the message
   */
  void make_stamp(string peer_id);

  /* is_broadcast:
     - returns whether this message should be passed along the rings or if it's
       just a one-time signal

     - by default, returns false
   */
  virtual bool is_broadcast();

  const Type type; // type of message, used for serialization (see individual
                   // classes for details)
  string stamp; // unique ID used to recognize the message after broadcasting it

protected:
  Message(Type _type);
};


#endif
