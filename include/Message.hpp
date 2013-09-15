#ifndef MESSAGE_H
#define MESSAGE_H

#include <exception>

#include "Config.hpp"

struct MessageParseException : public std::exception {
  const char* what() const throw()
  { return "Could not parse message"; }
};

struct Message
{
  typedef MessageType Type;

  virtual ~Message();

  virtual string serialize();
  virtual void display();

  void make_stamp(string peer_id);
  void set_stamp(string _stamp);

  bool is_broadcast();


  const Type type;
  string stamp;

protected:
  Message(Type _type);
};


#endif
