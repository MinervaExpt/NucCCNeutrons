//File: MuonZMomentum.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/MuonZMomentum.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  MuonZMomentum::MuonZMomentum(const YAML::Node& config): Cut(config), fMin(config["min"].as<GeV>())
  {
  }

  bool MuonZMomentum::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetTruthPmu().p().z() > fMin;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::MuonZMomentum> MuonZMomentum_reg("MuonZMomentum");
}
