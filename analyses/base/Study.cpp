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
               std::map<std::string, std::vector<evt::Universe*>>& /*universes*/): fPasses(std::move(mustPass))
  {
  }

  Study::~Study()
  {
  }

  bool Study::passesCuts(const evt::Universe& event)
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

  //Default case: assume that Universes cannot be assumed to have the same behavior.  This Study might make additional cuts for example.
  //Achieves backwards compatibility for the vast majority of Studies that aren't performance-critical and keeps things simple for simple
  //use cases.
  void Study::mcSignal(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt)
  {
    for(const auto univ: univs) mcSignal(*univ, model.GetWeight(*univ, evt));
  }

  void Study::mcBackground(const std::vector<evt::Universe*>& univs, const background_t& background, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt)
  {
    for(const auto univ: univs) mcBackground(*univ, background, model.GetWeight(*univ, evt));
  }

  void Study::truth(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt)
  {
    for(const auto univ: univs) truth(*univ, model.GetWeight(*univ, evt));
  }
}
