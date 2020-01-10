//File: Q3Range.cpp
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/Q3Range.h"

namespace truth
{
  Q3Range::Q3Range(const YAML::Node& config): fMin(config["min"].as<GeV>(0_GeV)), fMax(config["min"].as<GeV>())

  bool passesCut(const CVUniverse& event) const
  {
    return event.GetTruthQ3() > fMin && event.GetTruthQ3() < fMax;
  }
}

namespace
{
  static plgn::Registrar<truth::Cut, truth::Q3Range> MainAnalysis_reg("Q3Range");
}
