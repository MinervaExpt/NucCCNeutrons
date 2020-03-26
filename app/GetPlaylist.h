//File: GetPlaylist.h
//Brief: A function that looks at an AnaTuple file and figures out the playlist name for the first event.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APP_GETPLAYLIST_H
#define APP_GETPLAYLIST_H

//c++ includes
#include <string>

namespace app
{
  std::string GetPlaylist(const std::string& fileName, const bool isMC);
}

#endif //APP_GETPLAYLIST_H
