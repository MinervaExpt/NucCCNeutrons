//File: TrackAngle.cpp
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/TrackAngle.h"

namespace reco
{
  TrackAngle::TrackAngle(const YAML::Node& config, const std::string& name): Cut(config, name), fMax(config["max"].as<degrees>())
  {
  }

  bool TrackAngle::checkCut(const evt::CVUniverse& event, PlotUtils::detail::empty& /*empty*/) const
  {
    return event.GetMuonTheta() <= fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::TrackAngle> TrackAngle_reg("TrackAngle");
}
