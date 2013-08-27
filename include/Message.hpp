#ifndef MESSAGE_H
#define MESSAGE_H

#include <exception>

#include "Config.hpp"

struct MessageParseException : public std::exception {
  const char* what() const throw()
  { return "Could not parse message"; }
};

class Message
{
public:
  typedef MessageType Type;

  virtual ~Message();

  virtual string serialize();
  virtual void display() = 0;

  void make_stamp(string peer_id);
  void set_stamp(string _stamp);

  const Type get_type();

  bool is_broadcast();

protected:
  Message(Type _type);
  Type type;
  string stamp;
};


#endif
