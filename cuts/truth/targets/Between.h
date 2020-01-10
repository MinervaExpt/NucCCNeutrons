//File: Between.h
//Brief: Require that a CVUniverse has a vertex truthnstructed between 2 nuclear targets.
//       Useful for defining the plastic sideband.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_BETWEEN_H
#define TRUTH_BETWEEN_H

//units includes
#include "units/NucCCNeutronsUnits.h"

//cut includes
#include "cuts/truth/Cut.h"

namespace truth
{
  class Between: public Cut
  {
    public:
      Between(const YAML::Node& config);
      virtual ~Between() = default;

    protected:
      virtual bool passesCut(const CVUniverse& event) const override;

    private:
      const mm fZMin;
      const mm fZMax;
  };
}

#endif //TRUTH_BETWEEN_H