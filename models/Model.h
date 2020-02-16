//File: Model.h
//Brief: A Model reweights the central value Monte Carlo sample by a multiplicative
//       constant.  It can be loaded from a YAML file.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_MODEL_H
#define MODEL_MODEL_H

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//utilities includes
#include "util/units.h"
#include "util/Factory.cpp"

namespace evt
{
  class CVUniverse;
}

namespace model
{
  class Model
  {
    public:
      Model(const YAML::Node& config);
      virtual ~Model() = default;

      virtual events GetWeight(const evt::CVUniverse& univ) const = 0;

    template <class DERIVED>
    using Registrar = plgn::Registrar<Model, DERIVED>;
  };
}

#endif //MODEL_MODEL_H
