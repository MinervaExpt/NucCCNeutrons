//File: ExactMatch.h
//Brief: Cut on a ExactMatch<> in some VARIABLE.  Minimizes the number of ways I
//       can do stupid things like swapping minimum and maximum.  Also
//       an opportunity to centralize N-1 Cuts infrastructure.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_UPPERLIMIT_H
#define TRUTH_UPPERLIMIT_H

//cut includes
#include "cuts/truth/Cut.h"

namespace truth
{
  template <class VARIABLE>
  class ExactMatch: public Cut
  {
    private:
      using UNIT = decltype(std::declval<VARIABLE>().truth(std::declval<evt::Universe>()));

    public:
      ExactMatch(const YAML::Node& config, const std::string& name): Cut(config, name), fEquals(config["equals"].as<UNIT>()), fVar(config["variable"])
      {
      }

      virtual ~ExactMatch() = default;

    protected:
      virtual bool passesCut(const evt::Universe& event) const override
      {
        return fVar.truth(event) == fEquals;
      }

    private:
      UNIT fEquals;
      VARIABLE fVar;
  };
}

#endif //TRUTH_UPPERLIMIT_H
