#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>

#include "Config.hpp"

#include "Message.hpp"

#ifdef DEBUG_ON
#define DEBUG(x) do { std::cerr << "DBG: " << x << std::endl; } while (0)
#else
#define DEBUG(x)
#endif


void parse_input(string& input, string& command, string& argument);
string itos(int i);

Message* parse_message(string msg);

#endif
