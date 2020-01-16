//File: OneSectionTarget.cpp
//Brief: Some of MINERvA's nuclear targets are a solid block of one material.
//       A OneSectionTarget Cut removes CVUniverses that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//
//       There is no reco cut for a one-section target, so this file is just
//       a place-holder.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/targets/OneSectionTarget.h"

namespace reco
{
  OneSectionTarget::OneSectionTarget(const YAML::Node& config): Cut(config)
  {
  }

  bool OneSectionTarget::passesCut(const evt::CVUniverse& /*event*/) const
  {
    return true;
  }
}

namespace
{
  static plgn::Registrar<reco::Cut, reco::OneSectionTarget> Target4Reco_reg("Target4Carbon");

  static plgn::Registrar<reco::Cut, reco::OneSectionTarget> WaterReco_reg("Water");
}
