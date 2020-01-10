//File: MuonMomentum.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/MuonMomentum.h"

namespace truth
{
  MuonMomentum::MuonMomentum(const YAML::Node& config): fMin(config["min"].as<GeV>())
  {
  }

  bool passesCut(const CVUniverse& event) const
  {
    return event.GetTruthMuonMomentum4V() > fMin;
  }
}

namespace
{
  static plgn::Registrar<truth::Cut, truth::MuonMomentum> MainAnalysis_reg("MuonMomentum");
}
