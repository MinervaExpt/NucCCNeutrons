//File: MINOSEfficiency.h
//Brief: A MINOSEfficiency model modifies the CV's weight to account for a data-MC
//       discrepancy in efficiency to reconstruct a muon in MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_MINOSEFFICIENCY_H
#define MODEL_MINOSEFFICIENCY_H

//models includes
#include "models/Model.h"

namespace model
{
  class MINOSEfficiency: public Model
  {
    public:
      MINOSEfficiency(const YAML::Node& config);
      virtual ~MINOSEfficiency() = default;

      virtual events GetWeight(const evt::CVUniverse& univ) const override;
  };
}

#endif //MODEL_MINOSEFFICIENCY_H
