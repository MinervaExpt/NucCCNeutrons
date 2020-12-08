//File: WithNoArgs.cpp
//Brief: A Model WithNoArgs just calls a function from DefaultUniverse with no
//       arguments needed.  Right now, MnvGENIEv1 is implemented this way.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "models/WithNoArgs.h"

//evt includes
#include "evt/Universe.h"

namespace model
{
  template <double(evt::Universe::*reweight)() const>
  WithNoArgs<reweight>::WithNoArgs(const YAML::Node& config): Model(config)
  {
  }

  template <double(evt::Universe::*reweight)() const>
  events WithNoArgs<reweight>::GetWeight(const evt::Universe& univ) const
  {
    return (univ.*reweight)();
  }
}

//Register some models from PlotUtils that I want to compose at runtime
namespace
{
  static model::WithNoArgs<&evt::Universe::GetRPAWeight>::Registrar reg_RPA("RPA");
  static model::WithNoArgs<&evt::Universe::GetLowRecoil2p2hWeight>::Registrar reg_2p2h("2p2h");
}
