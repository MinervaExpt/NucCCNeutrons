//File: GENIEUncertainties.h
//Brief: A GENIEUncertainties model modifies the CV's weight to account for
//       uncertainties in parameters of our neutrino event generator.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_GENIEUNCERTAINTIES_H
#define MODEL_GENIEUNCERTAINTIES_H

//models includes
#include "models/Model.h"

namespace model
{
  class GENIEUncertainties: public Model
  {
    public:
      GENIEUncertainties(const YAML::Node& config);
      virtual ~GENIEUncertainties() = default;

      virtual events GetWeight(const evt::CVUniverse& univ) const override;
  };
}

#endif //MODEL_GENIEUNCERTAINTIES_H
