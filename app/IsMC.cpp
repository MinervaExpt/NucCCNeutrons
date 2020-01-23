//File: IsMC.cpp
//Brief: Decide whether a MINERvA AnaTuple file is data or MC.
//       At the time of writing, this just looks for the Truth
//       tree.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "app/IsMC.h"

//ROOT includes
#include "TFile.h"

//c++ includes
#include <stdexcept>

namespace app
{
  bool IsMC(const std::string& fileName)
  {
    auto file = TFile::Open(fileName.c_str()); //xrootd-compatible
    if(!file)
    {
      throw std::runtime_error("Failed to open file named " + fileName
                               + " to check whether it's from data or MC.");
    }

    return (file->FindKey("Truth") != nullptr);
  }
}
