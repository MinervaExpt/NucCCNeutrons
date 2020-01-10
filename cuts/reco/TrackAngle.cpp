//File: TrackAngle.cpp
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/TrackAngle.h"

namespace reco
{
  TrackAngle::TrackAngle(const YAML::Node& config): fMax(config["max"].as<GeV>())
  {
  }

  bool passesCut(const CVUniverse& event) const
  {
    return event.GetMuonMomentum4V().theta() > fMax;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, truth::TrackAngle> MainAnalysis_reg("TrackAngle");
}
