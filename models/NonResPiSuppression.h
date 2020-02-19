//File: NonResPiSuppression.h
//Brief: A NonResPiSuppression model modifies the CV's weight to account for a better-modeled or better-constrained
//       flux.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_FLUX_H
#define MODEL_FLUX_H

//models includes
#include "models/Model.h"

namespace model
{
  class NonResPiSuppression: public Model
  {
    public:
      NonResPiSuppression(const YAML::Node& config);
      virtual ~NonResPiSuppression() = default;

      virtual events GetWeight(const evt::CVUniverse& univ) const override;
  };
}

#endif //MODEL_FLUX_H
