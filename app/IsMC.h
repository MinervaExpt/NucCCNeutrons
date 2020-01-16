//File: IsMC.h
//Brief: A function to determine whether I'm processing a MINERvA MC file.
//       At the time of writing, I just check for the Truth tree.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_ISMC_H
#define APO_ISMC_H

//c++ includes
#include <string>

namespace apo
{
  bool IsMC(const std::string& fileName);
}

#endif //APO_ISMC_H
