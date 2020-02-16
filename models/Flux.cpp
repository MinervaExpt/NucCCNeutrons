//File: Flux.cpp
//Brief: A Flux model modifies the CV's weight to account for a better-modeled or better-constrained
//       flux.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//models includes
#include "models/Flux.h"

//evt includes
#include "evt/CVUniverse.h"

namespace model
{
  Flux::Flux(const YAML::Node& config): Model(config)
  {
  }

  events Flux::GetWeight(const evt::CVUniverse& univ) const
  {
    return univ.GetFluxAndCVWeight();
  }
}

namespace
{
  model::Model::Registrar<model::Flux> reg_flux("FluxAndCV");
}
