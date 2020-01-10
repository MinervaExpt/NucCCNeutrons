//File: ThreeSectionTarget.h
//Brief: One of MINERvA's nuclear targets is split into 3 sections of different
//       materials.  A ThreeSectionTarget Cut removes CVUniverses that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_THREESECTIONTARGET_H
#define RECO_THREESECTIONTARGET_H

//cut includes
#include "cuts/reco/Cut.h"

//ROOT includes
#include "Math/AxisAngle.h"

namespace evt
{
  class CVUniverse;
}

namespace reco
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
        impl<MaterialZ>::check(event, fRotation, mmToDivision);
      }

    private:
      const ROOT::Math::AxisAngle fRotation;

      //Implementation details to reuse code
      //Case: Iron
      template <int Z>
      struct impl
      {
        inline static bool check(const evt::CVUniverse::vertex_t& vertex, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
        {
          return vtx.x() < 0;
        }
      };

      //Special handling for Carbon
      template <>
      struct impl<6>
      {
        inline static bool check(const evt::CVUniverse::vertex_t& vertex, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
        {
          const auto local = fRotation * event.GetVtx();
          return (local.y() - mmToDivide) > 0;
        }
      };

      //Special handling for Lead
      template <int Z>
      struct impl<82>
      {
        inline static bool check(const evt::CVUniverse::vertex_t& vertex, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
        {
          return vtx.x() > 0;
        }
      };
  };
}

#endif //RECO_THREESECTIONTARGET_H
