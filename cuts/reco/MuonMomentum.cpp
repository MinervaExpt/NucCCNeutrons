//File: MuonMomentum.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/MuonMomentum.h"

namespace reco
{
  MuonMomentum::MuonMomentum(const YAML::Node& config): fMin(config["min"].as<GeV>())
  {
  }

  bool passesCut(const CVUniverse& event) const
  {
    return event.GetMuonMomentum4V() > fMin;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::MuonMomentum> MainAnalysis_reg("MuonMomentum");
}
