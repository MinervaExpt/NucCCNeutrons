//File: Study.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//background includes
#include "analyses/base/Background.h"

//cuts includes
#include "cuts/reco/Cut.h"

//c++ includes
#include <algorithm>

namespace ana
{
  Study::Study(const YAML::Node& /*config*/, util::Directory& /*dir*/, cuts_t&& mustPass,
               const std::vector<background_t>& /*backgrounds*/,
               std::map<std::string, std::vector<evt::CVUniverse*>>& /*universes*/): fPasses(std::move(mustPass))
  {
  }

  Study::~Study()
  {
  }

  bool Study::passesCuts(const evt::CVUniverse& event)
  {
    PlotUtils::detail::empty dummy;
    return std::all_of(fPasses.begin(), fPasses.end(), [&event, &dummy](auto& cut) { return cut->passesCut(event, dummy); });
  }

  void Study::afterAllFiles(const events /*passedSelection*/)
  {
  }

  bool Study::wantsTruthLoop() const
  {
    return true;
  }
}
