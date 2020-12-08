//File: IsInTarget.h
//Brief: Require that a Universe is in a given nuclear target.  Just makes a vertex z cut in truth.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_ISINTARGET_H
#define TRUTH_ISINTARGET_H

//units includes
#include "util/units.h"

//cut includes
#include "cuts/truth/Cut.h"

namespace truth
{
  class IsInTarget: public Cut
  {
    public:
      IsInTarget(const YAML::Node& config, const std::string& name);
      virtual ~IsInTarget() = default;

    protected:
      virtual bool passesCut(const evt::Universe& event) const override;

    private:
      const mm fZMin;
      const mm fZMax;
  };
}

#endif //TRUTH_ISINTARGET_H
