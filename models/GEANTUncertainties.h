//File: GEANTUncertainties.h
//Brief: A GEANTUncertainties model modifies the CV's weight to account for
//       uncertainties in parameters of our particle transport simulation.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_GEANTUNCERTAINTIES_H
#define MODEL_GEANTUNCERTAINTIES_H

//models includes
#include "models/Model.h"

namespace model
{
  class GEANTUncertainties: public Model
  {
    public:
      GEANTUncertainties(const YAML::Node& config);
      virtual ~GEANTUncertainties() = default;

      virtual events GetWeight(const evt::Universe& univ) const override;
  };
}

#endif //MODEL_GEANTUNCERTAINTIES_H
