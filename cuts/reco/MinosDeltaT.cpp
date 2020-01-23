//File: MinosDeltaT.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/MinosDeltaT.h"

namespace reco
{
  MinosDeltaT::MinosDeltaT(const YAML::Node& config, const std::string& name): Cut(config, name), fMax(config["max"].as<ns>())
  {
  }

  bool MinosDeltaT::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetMINOSTrackDeltaT() < fMax;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::MinosDeltaT, std::string&> MainAnalysis_reg("MinosDeltaT");
}
