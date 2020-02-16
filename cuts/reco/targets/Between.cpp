//File: Between.cpp
//Brief: Require that a CVUniverse has a vertex reconstructed between 2 nuclear targets.
//       Useful for defining the plastic sideband.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//targets includes
#include "cuts/reco/targets/Between.h"

namespace reco
{
  Between::Between(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                       fZMin(config["us"]["reco"]["max"].as<mm>()),
                                                                       fZMax(config["ds"]["reco"]["min"].as<mm>())
  {
  }

  bool Between::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetVtx().z() > fZMin && event.GetVtx().z() < fZMax;
  }
}

//Register Between as a kind of Cut
namespace
{
  static reco::Cut::Registrar<reco::Between> RecoBetween_reg("Between");
}
