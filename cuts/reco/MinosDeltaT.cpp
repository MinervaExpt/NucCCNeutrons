//File: MinosDeltaT.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/MinosDeltaT.h"

namespace reco
{
  MinosDeltaT::MinosDeltaT(const YAML::Node& config): fMax(config["max"].as<GeV>())
  {
  }

  bool passesCut(const CVUniverse& event) const
  {
    return event.GetMinosDeltaT() > fMax;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::MinosDeltaT> MainAnalysis_reg("MinosDeltaT");
}
