//File: IsInTarget.cpp
//Brief: Require that a CVUniverse is in a given nuclear target.  Just makes a vertex z cut in both
//       reconstruction and truth.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//targets includes
#include "cuts/truth/targets/IsInTarget.h"

namespace truth
{
  IsInTarget::IsInTarget(const YAML::Node& config): fZMin(config["truth"]["min"].as<double>()),
                                                    fZMax(config["truth"]["max"].as<double>()),
  {
  }

  bool IsInTarget::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetTruthVtx().z() > fZMin && event.GetTruthVtx().z() < fZMax;
  }
  }
}

//Register IsInTarget as a kind of Cut
namespace
{
  static plgn::Registrar<truth::Cut, truth::IsInTarget> MainAnalysis_reg("IsInTarget");
}
