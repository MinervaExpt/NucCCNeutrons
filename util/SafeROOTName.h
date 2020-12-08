//File: SafeROOTName.h
//Brief: Turns a std::string into a name that the user can
//       easily interact with from the Cling interpretter.
//       Replaces decimal points and spaces with _s to
//       accomplish this.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

#ifndef UTIL_SAFEROOTNAME_H
#define UTIL_SAFEROOTNAME_H

//c++ includes
#include <string>

namespace util
{
  std::string SafeROOTName(std::string copy);
}

#endif //UTIL_SAFEROOTNAME_H
