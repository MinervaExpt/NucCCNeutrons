//File: MuonMomentum.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/MuonMomentum.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  MuonMomentum::MuonMomentum(const YAML::Node& config, const std::string& name): Cut(config, name), fMin(config["min"].as<GeV>()), fMax(config["max"].as<GeV>())
  {
  }

  bool MuonMomentum::passesCut(const evt::Universe& event) const
  {
    const auto pMu = event.GetTruthPmu().p().mag();
    return pMu > fMin && pMu < fMax;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::MuonMomentum> MuonMomentum_reg("MuonMomentum");
}
