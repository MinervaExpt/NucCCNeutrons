//File: arachne.h
//Brief: arachne() maps a SliceID to a link to MINERvA's online event
//       display.  It's useful for debugging.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UTIL_ARACHNE_H
#define UTIL_ARACHNE_H

//c++ includes
#include <string>

namespace evt
{
  struct SliceID;
}

namespace util
{
  std::string arachne(const evt::SliceID& id, const bool isData, const bool useRodriges = false);
}

#endif //UTIL_ARACHNE_H
