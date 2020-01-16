//File: TrackAngle.cpp
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/TrackAngle.h"

namespace reco
{
  TrackAngle::TrackAngle(const YAML::Node& config): Cut(config), fMax(config["max"].as<double>())
  {
  }

  bool TrackAngle::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetMuonP().p().theta() < fMax;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::TrackAngle> MainAnalysis_reg("TrackAngle");
}
