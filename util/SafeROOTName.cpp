//File: SafeROOTName.cpp
//Brief: Turns a std::string into a name that the user can
//       easily interact with from the Cling interpretter.
//       Replaces decimal points and spaces with _s to
//       accomplish this.
//Author: Andrew Olivier aolivier@ur.rochester.edu 

//NucCCNeutrons includes
#include "util/SafeROOTName.h"

namespace util
{
  std::string SafeROOTName(std::string copy)
  {
    //Replace decimal points with underscores
    if(copy.find_first_of(".") != std::string::npos) copy.replace(copy.find_first_of("."), 1, "_");
  
    //Replace spaces with underscores
    size_t spacePos = std::string::npos;
    while((spacePos = copy.find_first_of(" ")) != std::string::npos)
    {
      copy.replace(spacePos, 1, "_");
    }
  
    //Replace characters reserved in c++ with underscores
    size_t specialPos = std::string::npos;
    while((specialPos = copy.find_first_of("+-/.*&<>,{}()^|\\")) != std::string::npos)
    {
      copy.replace(specialPos, 1, "_");
    }
  
    return copy;
  }
}
