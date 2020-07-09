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
#include "Math/Vector3D.h"

namespace evt
{
  class CVUniverse;
}

namespace reco
{
  namespace detail
  {
    //Implementation details to reuse code
    //Case: Iron
    template <int Z>
    struct impl
    {
      inline static bool check(const units::LorentzVector<mm>& vertex, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
      {
        const mm local = (rot * (vertex.p().in<mm>())).y();
        return vertex.x() < 0_mm && (local - mmToDivide) < 0_mm;
      }
    };

    //Special handling for Carbon
    template <>
    struct impl<6>
    {
      inline static bool check(const units::LorentzVector<mm>& vertex, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
      {
        const mm local = (rot * (vertex.p().in<mm>())).y();
        return (local - mmToDivide) > 0_mm;
      }
    };

    //Special handling for Lead
    template <>
    struct impl<82>
    {
      inline static bool check(const units::LorentzVector<mm>& vertex, const ROOT::Math::AxisAngle& rot, const mm mmToDivide)
      {
        return vertex.x() > 0_mm && !impl<6>::check(vertex, rot, mmToDivide);
      }
    };
  }

  template <int MaterialZ>
  class ThreeSectionTarget: public Cut
  {
    private:
      const double rotationAngle = M_PI/6.;
      const mm mmToDivide = 0_mm;

    public:
      ThreeSectionTarget(const YAML::Node& config, const std::string& name): Cut(config, name), fRotation(ROOT::Math::XYZVector(0., 0., 1.), rotationAngle)
      {
      }

      virtual ~ThreeSectionTarget() = default;

    protected:
      virtual bool passesCut(const evt::CVUniverse& event) const override
      {
        return detail::impl<MaterialZ>::check(event.GetVtx(), fRotation, mmToDivide);
      }

    private:
      const ROOT::Math::AxisAngle fRotation;
  };
}

#endif //RECO_THREESECTIONTARGET_H
