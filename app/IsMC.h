//File: IsMC.h
//Brief: A function to determine whether I'm processing a MINERvA MC file.
//       At the time of writing, I just check for the Truth tree.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APP_ISMC_H
#define APP_ISMC_H

//c++ includes
#include <string>

namespace app
{
  bool IsMC(const std::string& fileName);
}

#endif //APP_ISMC_H
