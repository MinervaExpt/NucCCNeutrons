//File: Range.h
//Brief: Cut on a Range<> in some VARIABLE.  Minimizes the number of ways I
//       can do stupid things like swapping minimum and maximum.  Also
//       an opportunity to centralize N-1 Cuts infrastructure.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_RANGE_H
#define TRUTH_RANGE_H

//cut includes
#include "cuts/truth/Cut.h"

namespace truth
{
  template <class VARIABLE>
  class Range: public Cut
  {
    private:
      using UNIT = decltype(std::declval<VARIABLE>().truth(std::declval<evt::Universe>()));

    public:
      Range(const YAML::Node& config, const std::string& name): Cut(config, name), fVar(config["variable"]),
                                                                fMin(config["min"].as<UNIT>(std::numeric_limits<typename UNIT::floating_point>::min())),
                                                                fMax(config["max"].as<UNIT>(std::numeric_limits<typename UNIT::floating_point>::max()))
      {
        if(config["equals"])
        {
          fMin = config["equals"].as<UNIT>();
          fMax = config["equals"].as<UNIT>();
        }
      }

      virtual ~Range() = default;

    protected:
      virtual bool passesCut(const evt::Universe& event) const override
      {
        const UNIT var = fVar.truth(event);
        return var >= fMin && var <= fMax;
      }

    private:
      VARIABLE fVar;
      UNIT fMin;
      UNIT fMax;
  };
}

#endif //TRUTH_RANGE_H
