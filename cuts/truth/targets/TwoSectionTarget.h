//File: TwoSectionTarget.h
//Brief: Some of MINERvA's nuclear targets are split into 2 sections of different
//       materials.  A TwoSectionTarget Cut removes Universes that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TWOSECTIONTARGET_H
#define TRUTH_TWOSECTIONTARGET_H

//cut includes
#include "cuts/truth/Cut.h"

//ROOT includes
#include "Math/AxisAngle.h"

namespace truth
{
  template <int MaterialZ>
  class TwoSectionTarget: public Cut
  {
    public:
      TwoSectionTarget(const YAML::Node& config, const std::string& name): Cut(config, name) {}

      virtual ~TwoSectionTarget() = default;

    protected:
      virtual bool passesCut(const evt::Universe& event) const override
      {
        return event.GetTruthTargetZ() == MaterialZ;
      }
  };
}

#endif //TRUTH_TWOSECTIONTARGET_H
