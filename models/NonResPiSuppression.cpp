//File: NonResPiSuppression.cpp
//Brief: A NonResPiSuppression model modifies the CV's weight to account for a better-modeled or better-constrained
//       flux.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//models includes
#include "models/NonResPiSuppression.h"

//evt includes
#include "evt/CVUniverse.h"

namespace model
{
  NonResPiSuppression::NonResPiSuppression(const YAML::Node& config): Model(config)
  {
    PlotUtils::DefaultCVUniverse::SetNonResPiReweight(true);
  }

  events NonResPiSuppression::GetWeight(const evt::CVUniverse& /*univ*/) const
  {
    return 1.; //Dummy weight.  The real work was done by
               //calling a static function in the constructor.
  }
}

namespace
{
  model::Model::Registrar<model::NonResPiSuppression> reg_flux("NonResPiSuppression");
}
