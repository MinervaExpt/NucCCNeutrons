//File: ExactMatch.h
//Brief: Cut on a ExactMatch<> in some VARIABLE.  Minimizes the number of ways I
//       can do stupid things like swapping minimum and maximum.  Also
//       an opportunity to centralize N-1 Cuts infrastructure.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_EXACTMATCH_H
#define TRUTH_EXACTMATCH_H

//cut includes
#include "cuts/reco/Cut.h"

namespace truth
{
  template <class UNIT, UNIT(evt::Universe::*reco)() const>
  class ExactMatch: public Cut
  {
    public:
      ExactMatch(const YAML::Node& config, const std::string& name): Cut(config, name), fEquals(config["equals"].as<UNIT>())
      {
      }

      virtual ~ExactMatch() = default;

    protected:
      virtual bool passesCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override
      {
        return (event.*truth)() == fEquals;
      }

    private:
      UNIT fEquals;
  };
}

#endif //TRUTH_EXACTMATCH_H
