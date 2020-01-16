//File: TwoSectionTarget.h
//Brief: Some of MINERvA's nuclear targets are split into 2 sections of different
//       materials.  A TwoSectionTarget Cut removes CVUniverses that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TWOSECTIONTARGET_H
#define RECO_TWOSECTIONTARGET_H

//cut includes
#include "cuts/reco/Cut.h"

//ROOT includes
#include "Math/AxisAngle.h"
#include "Math/Vector3D.h"

namespace evt
{
  class CVUniverse;
}

namespace reco
{
  namespace detail
  {
    //Implementation details to make material decision at compile-time
    //Base case for top material
    template <bool isTop>
    struct impl
    {
      inline static bool check(const mm distToDivide) { return distToDivide > 0_mm; }
    };
                                                                                   
    //Special case for bottom material
    template <>
    struct impl<false>
    {
      inline static bool check(const mm distToDivide) { return distToDivide < 0_mm; }
    };
  }

  template <bool isTop, int rotationSign>
  class TwoSectionTarget: public Cut
  {
    private:
      static constexpr auto rotationAngle = M_PI/3.;
      static constexpr auto mmToDivide = 205_mm;

    public:
      TwoSectionTarget(const YAML::Node& config): Cut(config), fRotation(ROOT::Math::XYZVector(0., 0., rotationSign), rotationAngle)
      {
      }

      virtual ~TwoSectionTarget() = default;

    protected:
      virtual bool passesCut(const evt::CVUniverse& event) const override
      {
        const mm distToCenter = (fRotation * event.GetVtx().p().in<mm>()).y();
        return detail::impl<isTop>::check(distToCenter - mmToDivide);
      }

    private:
      const ROOT::Math::AxisAngle fRotation;
  };
}

#endif //RECO_TWOSECTIONTARGET_H
