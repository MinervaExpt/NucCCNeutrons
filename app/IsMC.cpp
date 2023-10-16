//File: IsMC.cpp
//Brief: Decide whether a MINERvA AnaTuple file is data or MC.
//       At the time of writing, this just looks for the Truth
//       tree.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "app/IsMC.h"

//ROOT includes
#include "TFile.h"
#include "TKey.h"
#include "TTree.h"

//c++ includes
#include <stdexcept>
#include <memory>

namespace app
{
  bool IsMC(const std::string& fileName)
  {
    std::unique_ptr<TFile> file(TFile::Open(fileName.c_str())); //xrootd-compatible
    if(!file)
    {
      throw std::runtime_error("Failed to open file named " + fileName
                               + " to check whether it's from data or MC.");
    }

    const auto truthKey = file->FindKey("Truth");
    if(truthKey == nullptr) return false;
    return (dynamic_cast<TTree*>(truthKey->ReadObj())->GetEntries() > 0);
  }
}
