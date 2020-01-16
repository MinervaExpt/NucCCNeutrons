//File: MuonMomentum.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/MuonMomentum.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  MuonMomentum::MuonMomentum(const YAML::Node& config): Cut(config), fMin(config["min"].as<GeV>())
  {
  }

  bool MuonMomentum::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetTruthPmu().p().mag() > fMin;
  }
}

namespace
{
  static plgn::Registrar<truth::Cut, truth::MuonMomentum> MainAnalysis_reg("MuonMomentum");
}
