//File: WithNoArgs.h
//Brief: A Model WithNoArgs<> just calls a function from DefaultCVUniverse.  This turns out to be all of MnvGENIEv1.
//       This isn't the entirety of Model because at least the low Q2 pion suppression needs some parameters.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_WITHNOARGS_H
#define MODEL_WITHNOARGS_H

//model includes
#include "models/Model.h"

namespace model
{
  template <double(evt::CVUniverse::*reweight)() const>
  class WithNoArgs: public Model
  {
    public:
      WithNoArgs(const YAML::Node& config);
      virtual ~WithNoArgs() = default;

      virtual events GetWeight(const evt::CVUniverse& univ) const override;
  };
}

#endif //MODEL_WITHNOARGS_H
