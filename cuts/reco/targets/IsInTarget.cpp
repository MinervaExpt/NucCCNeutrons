//File: IsInTarget.cpp
//Brief: Require that a Universe is in a given nuclear target.  Just makes a vertex z reco in both
//       reconstruction.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//targets includes
#include "cuts/reco/targets/IsInTarget.h"

namespace reco
{
  IsInTarget::IsInTarget(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                             fZMin(config["reco"]["min"].as<mm>()),
                                                                             fZMax(config["reco"]["max"].as<mm>())
  {
  }

  bool IsInTarget::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    return event.GetVtx().z() > fZMin && event.GetVtx().z() < fZMax;
  }
}

//Register IsInTarget as a kind of Cut
namespace
{
  static reco::Cut::Registrar<reco::IsInTarget> IsInTarget_reg("IsInTarget");
}
