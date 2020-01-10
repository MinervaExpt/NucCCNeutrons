//File: IsInTarget.cpp
//Brief: Require that a CVUniverse is in a given nuclear target.  Just makes a vertex z reco in both
//       reconstruction and truth.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//targets includes
#include "cuts/reco/targets/IsInTarget.h"

namespace reco
{
  IsInTarget::IsInTarget(const YAML::Node& config): fZMin(config["reco"]["min"].as<double>()),
                                                    fZMax(config["reco"]["max"].as<double>())
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
  static plgn::Registrar<reco::Cut, reco::IsInTarget> MainAnalysis_reg("IsInTarget");
}
