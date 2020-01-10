//File: OneSectionTarget.h
//Brief: Some of MINERvA's nuclear targets are a solid block of one material.
//       A OneSectionTarget Cut removes CVUniverses that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//
//       There is no reco cut for a one-section target, so this file is just
//       a place-holder.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_ONESECTIONTARGET_H
#define RECO_ONESECTIONTARGET_H

//cut includes
#include "cuts/reco/Cut.h"

namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class OneSectionTarget: public Cut
  {
    public:
      OneSectionTarget(const YAML::Node& /*config*/) = default;

      virtual OneSectionTarget() = default;

    protected:
      virtual bool isReco(const evt::CVUniverse& event) const override;
  };
}

#endif //RECO_ONESECTIONTARGET_H
