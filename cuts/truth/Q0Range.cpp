//File: Q0Range.cpp
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/Q0Range.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  Q0Range::Q0Range(const YAML::Node& config): Cut(config), fMin(config["min"].as<GeV>(0_GeV)), fMax(config["max"].as<GeV>())
  {
  }

  bool Q0Range::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetTruthQ0() > fMin && event.GetTruthQ0() < fMax;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::Q0Range> Q0Range_reg("Q0Range");
}
