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

  virtual string serialize();
  virtual void display();

protected:
  Message(Type _type);
  Type type;
};


#endif
