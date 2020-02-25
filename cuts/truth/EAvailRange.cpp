//File: EAvailRange.cpp
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/EAvailRange.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  EAvailRange::EAvailRange(const YAML::Node& config): Cut(config), fMin(config["min"].as<GeV>(0_GeV)), fMax(config["max"].as<GeV>())
  {
  }

  bool EAvailRange::passesCut(const evt::CVUniverse& event) const
  {
    const GeV EAvail = event.GetTruthEAvailable();
    return EAvail > fMin && EAvail < fMax;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::EAvailRange> EAvailRange_reg("EAvailRange");
}
