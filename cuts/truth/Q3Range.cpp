//File: Q3Range.cpp
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/Q3Range.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  Q3Range::Q3Range(const YAML::Node& config): Cut(config), fMin(config["min"].as<GeV>(0_GeV)), fMax(config["max"].as<GeV>())
  {
  }

  bool Q3Range::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetTruthQ3() > fMin && event.GetTruthQ3() < fMax;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::Q3Range> Q3Range_reg("Q3Range");
}
