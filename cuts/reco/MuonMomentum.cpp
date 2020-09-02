//File: MuonMomentum.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/MuonMomentum.h"

namespace reco
{
  MuonMomentum::MuonMomentum(const YAML::Node& config, const std::string& name): Cut(config, name), fMin(config["min"].as<GeV>()), fMax(config["max"].as<GeV>())
  {
  }

  bool MuonMomentum::checkCut(const evt::CVUniverse& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const auto pMu = event.GetMuonP().p().mag();
    return pMu > fMin && pMu < fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::MuonMomentum> MuonMomentum_reg("MuonMomentum");
}
