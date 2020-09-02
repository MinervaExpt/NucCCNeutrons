//File: RecoilERange.cpp
//Brief: A Cut on a range of values in reco recoil energy.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/RecoilERange.h"

namespace reco
{
  RecoilERange::RecoilERange(const YAML::Node& config, const std::string& name): Cut(config, name), fMin(config["min"].as<GeV>(0_GeV)), fMax(config["max"].as<GeV>()), fCalc(config["EAvailable"])
  {
  }

  bool RecoilERange::checkCut(const evt::CVUniverse& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const auto reco = fCalc.reco(event);
    return reco > fMin && reco < fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::RecoilERange> RecoilERange_reg("RecoilERange");
}
