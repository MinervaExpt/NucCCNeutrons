//File: IsInTarget.h
//Brief: Require that a CVUniverse is in a given nuclear target.  Just makes a vertex z cut in both
//       reconstruction.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_ISINTARGET_H
#define RECO_ISINTARGET_H

//units includes
#include "units/NucCCNeutronsUnits.h"

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  class IsInTarget: public Cut
  {
    public:
      IsInTarget(const YAML::Node& config);
      virtual ~IsInTarget() = default;

    protected:
      virtual bool passesCut(const CVUniverse& event) const override;

    private:
      const mm fZMin;
      const mm fZMax;
  };
}

#endif //RECO_ISINTARGET_H
