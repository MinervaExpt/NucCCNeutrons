//File: IsInTarget.cpp
//Brief: Require that a Universe is in a given nuclear target.  Just makes a vertex z cut in truth.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//targets includes
#include "cuts/truth/targets/IsInTarget.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  IsInTarget::IsInTarget(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                             fZMin(config["truth"]["min"].as<mm>()),
                                                                             fZMax(config["truth"]["max"].as<mm>())
  {
  }

  bool IsInTarget::passesCut(const evt::Universe& event) const
  {
    return event.GetTruthVtx().z() > fZMin && event.GetTruthVtx().z() < fZMax;
  }
}

//Register IsInTarget as a kind of Cut
namespace
{
  static truth::Cut::Registrar<truth::IsInTarget> IsInTarget_reg("IsInTarget");
}
