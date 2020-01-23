//File: IsInTarget.cpp
//Brief: Require that a CVUniverse is in a given nuclear target.  Just makes a vertex z reco in both
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

  bool IsInTarget::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetVtx().z() > fZMin && event.GetVtx().z() < fZMax;
  }
}

//Register IsInTarget as a kind of Cut
namespace
{
  static plgn::Registrar<reco::Cut, reco::IsInTarget, std::string&> MainAnalysis_reg("IsInTarget");
}
