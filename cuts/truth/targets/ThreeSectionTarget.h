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
#include "cuts/truth/Cut.h"

//ROOT includes
#include "Math/AxisAngle.h"

//utilities includes
#include "util/units.h"

//ROOT includes for rotations
#include "Math/Vector3D.h"

namespace evt
{
  class CVUniverse;
}

//metaprogramming details that have to go at namespace scope :(
namespace
{
  //Implementation details to reuse code
  //Case: Iron and lead
  template <int Z>
  struct Impl
  {
    inline static bool check(const evt::CVUniverse& event, const ROOT::Math::AxisAngle& /*rot*/, const mm /*mmToDivide*/)
    {
      return event.GetTruthTargetZ() == Z;
    }
  };

  //Special handling for Carbon
  template <>
  struct Impl<6l>
  {
    inline static bool check(const evt::CVUniverse& event, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
    {
      const auto local = rot * (event.GetTruthVtx().p().in<mm>());
      return (event.GetTruthTargetZ() == 6) && (mm(local.y()) - mmToDivide) > 0_mm;
    }
  };
}

namespace truth
{
  template <int MaterialZ>
  class ThreeSectionTarget: public Cut
  {
    private:
      const double rotationAngle = M_PI/6.;
      const mm mmToDivide = 0_mm;

    public:
      ThreeSectionTarget(const YAML::Node& config): Cut(config), fRotation(ROOT::Math::XYZVector(0., 0., 1.), rotationAngle)
      {
      }

      virtual ~ThreeSectionTarget() = default;

    protected:
      ROOT::Math::AxisAngle fRotation;

      virtual bool passesCut(const evt::CVUniverse& event) const override
      {
        return ::Impl<MaterialZ>::check(event, fRotation, mmToDivide);
      }
  };
}

#endif //TRUTH_THREESECTIONTARGET_H
