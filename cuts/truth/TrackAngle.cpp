//File: TrackAngle.cpp
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/TrackAngle.h"

namespace truth
{
  TrackAngle::TrackAngle(const YAML::Node& config): fMax(config["max"].as<GeV>())
  {
  }

  bool passesCut(const CVUniverse& event) const
  {
    return event.GetTruthMuonMomentum4V().theta() > fMax;
  }
}

namespace
{
  static plgn::Registrar<truth::Cut, truth::TrackAngle> MainAnalysis_reg("TrackAngle");
}
