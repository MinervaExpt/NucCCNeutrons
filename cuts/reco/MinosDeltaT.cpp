//File: MinosDeltaT.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/MinosDeltaT.h"

namespace reco
{
  MinosDeltaT::MinosDeltaT(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                               fMin(config["min"].as<ns>()),
                                                                               fMax(config["max"].as<ns>())
  {
  }

  bool MinosDeltaT::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const ns deltaT = event.GetMINOSTrackDeltaT();
    return deltaT > fMin && deltaT < fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::MinosDeltaT> MinosDeltaT_reg("MinosDeltaT");
}
