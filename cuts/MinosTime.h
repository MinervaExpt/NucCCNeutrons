//File: MinosTime.h
//Brief: A Cut on the time difference between the MINERvA muon Track
//       and matched MINOS Track.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_MINOSTIME_H
#define APO_MINOSTIME_H

//Cuts includes
#include "Cuts/Cut.h"

//BaseUnits includes
#include "units/NucCCNeutronUnits.h"

namespace apo
{
  class MinosTime: public Cut
  {
    public:
      MinosTime(const YAML::Node& config);
      virtual ~MinosTime() = default;

    protected:
      virtual bool doCut(const SchemaView& event) override;

    private:
      const ns fMinTime;
      const ns fMaxTime;
  };
}

#endif //APO_MINOSTIME_H
