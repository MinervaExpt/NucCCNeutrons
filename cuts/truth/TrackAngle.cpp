//File: TrackAngle.cpp
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/TrackAngle.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  TrackAngle::TrackAngle(const YAML::Node& config, const std::string& name): Cut(config, name), fMax(config["max"].as<degrees>())
  {
  }

  bool TrackAngle::passesCut(const evt::Universe& event) const
  {
    return event.GetTruthPmu().p().theta() <= fMax;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::TrackAngle> TrackAngle_reg("TrackAngle");
}
