//File: Q3Range.cpp
//Brief: A Cut on a range of values in reco 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/Q3Range.h"

namespace reco
{
  Q3Range::Q3Range(const YAML::Node& config, const std::string& name): Cut(config, name), fMin(config["min"].as<GeV>(0_GeV)), fMax(config["max"].as<GeV>())
  {
  }

  bool Q3Range::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetQ3() > fMin && event.GetQ3() < fMax;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::Q3Range, std::string&> MainAnalysis_reg("Q3Range");
}
