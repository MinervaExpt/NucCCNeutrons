//File: OneSectionTarget.h
//Brief: Some of MINERvA's nuclear targets are a solid block of one material.
//       A OneSectionTarget Cut removes Universes that are not in
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
  class Universe;
}

namespace reco
{
  class OneSectionTarget: public Cut
  {
    public:
      OneSectionTarget(const YAML::Node& config, const std::string& name);

      virtual ~OneSectionTarget() = default;

    protected:
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override;
  };
}

#endif //RECO_ONESECTIONTARGET_H
