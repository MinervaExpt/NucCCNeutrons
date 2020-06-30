//File: Q3Range.cpp
//Brief: A Cut on a range of values in reco 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/Q3Range.h"

namespace reco
{
  Q3Range::Q3Range(const YAML::Node& config, const std::string& name): Cut(config, name), fCalc(config), fMin(config["min"].as<GeV>(0_GeV)), fMax(config["max"].as<GeV>())
  {
  }

  bool Q3Range::passesCut(const evt::CVUniverse& event) const
  {
    return fCalc.reco(event) > fMin && fCalc.reco(event) < fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::Q3Range> Q3Range_reg("Q3Range");
}
