//File: Flux.h
//Brief: A Flux model modifies the CV's weight to account for a better-modeled or better-constrained
//       flux.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_FLUX_H
#define MODEL_FLUX_H

//models includes
#include "models/Model.h"

namespace model
{
  class Flux: public Model
  {
    public:
      Flux(const YAML::Node& config);
      virtual ~Flux() = default;

      virtual events GetWeight(const evt::Universe& univ) const override;
  };
}

#endif //MODEL_FLUX_H
