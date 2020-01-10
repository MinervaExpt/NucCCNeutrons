//File: Between.cpp
//Brief: Require that a CVUniverse has a vertex truthnstructed between 2 nuclear targets.
//       Useful for defining the plastic sideband.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//targets includes
#include "cuts/truth/targets/Between.h"

namespace truth
{
  Between::Between(const YAML::Node& config): fZMin(config["us"]["truth"]["max"].as<mm>()),
                                              fZMax(config["ds"]["truth"]["min"].as<mm>())
  {
  }

  bool Between::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetTruthVtx().z() > fZMin && event.GetTruthVtx().z() < fZMax;
  }
}

//Register Between as a kind of Cut
namespace
{
  static plgn::Registrar<truth::Cut, truth::Between> MainAnalysis_reg("Between");
}
