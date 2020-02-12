//File: TrackAngle.cpp
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/TrackAngle.h"

namespace reco
{
  TrackAngle::TrackAngle(const YAML::Node& config, const std::string& name): Cut(config, name), fMax(config["max"].as<double>()/180.*M_PI)
  {
  }

  bool TrackAngle::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetMuonP().p().theta() < fMax;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::TrackAngle, std::string&> MainAnalysis_reg("TrackAngle");
}
