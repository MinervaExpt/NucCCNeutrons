//File: ThreeSectionTarget.h
//Brief: One of MINERvA's nuclear targets is split into 3 sections of different
//       materials.  A ThreeSectionTarget Cut removes CVUniverses that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_THREESECTIONTARGET_H
#define TRUTH_THREESECTIONTARGET_H

//cut includes
#include "cut/Cut.h"

//ROOT includes
#include "Math/AxisAngle.h"

namespace evt
{
  class CVUniverse;
}

namespace truth
{
  template <int MaterialZ>
  class ThreeSectionTarget: public Cut
  {
    private:
      constexpr auto rotationAngle = M_PI/6.;
      constexpr auto mmToDivide = 0_mm;

    public:
      ThreeSectionTarget(const YAML::Node& /*config*/): fRotation(ROOT::Math::XYZVector(0., 0., 1.), rotationAngle)
      {
      }

      virtual ThreeSectionTarget() = default;

    protected:
      virtual bool passesCut(const evt::CVUniverse& event) const override
      {
        Impl<MaterialZ>::check(event, fRotation, mmToDivision);
      }

    private:
      //Implementation details to reuse code
      //Case: Iron and lead
      template <int Z>
      struct Impl
      {
        inline static bool check(const evt::CVUniverse& event, const ROOT::Math::AxisAngle& /*rot*/, const mm /*mmToDivide*/)
        {
          return return event.GetTruthTargetZ() == MaterialZ;
        }
      };

      //Special handling for Carbon
      template <>
      struct Impl<6>
      {
        inline static bool check(const evt::CVUniverse& event, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
        {
          const auto local = rot * event.GetTruthVtx();
          return (event.GetTruthTargetZ() == 6) && (local.y() - mmToDivide) > 0;
        }
      };
  };
}

#endif //TRUTH_THREESECTIONTARGET_H
