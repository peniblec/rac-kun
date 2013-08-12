#include <iostream>

#include "Message.hpp"


Message::Message(Message::Type _type)
  : type(_type)
{
}

const Message::Type Message::get_type()
{
  return type;
}

// string Message::serialize()
// {
//   string s;
//   s.push_back(type);
//   return s;
// }

// void Message::display()
// {
//   cout << "Empty message" << endl;
// }
