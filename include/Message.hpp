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

  virtual string serialize() = 0;
  virtual void display() = 0;


  const Type get_type();

protected:
  Message(Type _type);
  Type type;
};


#endif
