//File: Between.h
//Brief: Require that a CVUniverse has a vertex reconstructed between 2 nuclear targets.
//       Useful for defining the plastic sideband.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_BETWEEN_H
#define RECO_BETWEEN_H

//units includes
#include "util/units.h"

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  class Between: public Cut
  {
    public:
      Between(const YAML::Node& config);
      virtual ~Between() = default;

    protected:
      virtual bool passesCut(const evt::CVUniverse& event) const override;

    private:
      const mm fZMin;
      const mm fZMax;
  };
}

#endif //RECO_BETWEEN_H
