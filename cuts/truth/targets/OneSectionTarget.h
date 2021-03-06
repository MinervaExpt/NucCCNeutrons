//File: OneSectionTarget.h
//Brief: Some of MINERvA's nuclear targets are a solid block of one material.
//       A OneSectionTarget Cut removes Universes that are not in
//       a specific section of a given target.  Since I don't need to change
//       MINERvA's geometry, this is just an implementation detail that I use
//       to define specific target sections.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_ONESECTIONTARGET_H
#define TRUTH_ONESECTIONTARGET_H

//cut includes
#include "cuts/truth/Cut.h"

//ROOT includes
#include "Math/AxisAngle.h"

namespace evt
{
  class Universe;
}

namespace truth
{
  namespace detail
  {
    //Implementation details to compare to multiple Zs
    template <int firstZ, int... MaterialZs>
    struct OR
    {
      inline static bool check(const int truthZ)
      {
        return (truthZ == firstZ) || OR<MaterialZs...>::check(truthZ);
      }
    };
                                                                       
    template <int lastZ>
    struct OR<lastZ>
    {
      inline static bool check(const int truthZ)
      {
        return truthZ == lastZ;
      }
    };
  }

  template <int ...MaterialZs>
  class OneSectionTarget: public Cut
  {
    public:
      OneSectionTarget(const YAML::Node& config, const std::string& name): Cut(config, name)
      {
      }

      virtual ~OneSectionTarget() = default;

    protected:
      virtual bool passesCut(const evt::Universe& event) const override
      {
        //TODO: Do I need to handle carbon differently?  I think the
        //      distance cut protects me from scintillator in target 4.
        return detail::OR<MaterialZs...>::check(event.GetTruthTargetZ());
      }
  };
}

#endif //TRUTH_ONESECTIONTARGET_H
