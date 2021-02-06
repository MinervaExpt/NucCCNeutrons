//File: ExactMatch.h
//Brief: Cut on a ExactMatch<> in some VARIABLE.  Minimizes the number of ways I
//       can do stupid things like swapping minimum and maximum.  Also
//       an opportunity to centralize N-1 Cuts infrastructure.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_UPPERLIMIT_H
#define RECO_UPPERLIMIT_H

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  template <class VARIABLE, class EVENT = PlotUtils::detail::empty>
  class ExactMatch: public Cut
  {
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::Universe>()));

    public:
      ExactMatch(const YAML::Node& config, const std::string& name): Cut(config, name), fEquals(config["equals"].as<UNIT>()), fVar(config["variable"])
      {
      }

      virtual ~ExactMatch() = default;

    protected:
      virtual bool checkCut(const evt::Universe& event, EVENT& /*notUsed*/) const override
      {
        return fVar.reco(event) == fEquals;
      }

    private:
      UNIT fEquals;
      VARIABLE fVar;
  };
}

#endif //RECO_UPPERLIMIT_H
