//File: WithNoArgs.cpp
//Brief: A Model WithNoArgs just calls a function from DefaultCVUniverse with no
//       arguments needed.  Right now, MnvGENIEv1 is implemented this way.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//model includes
#include "models/WithNoArgs.h"

//evt includes
#include "evt/CVUniverse.h"

namespace model
{
  template <double(evt::CVUniverse::*reweight)() const>
  WithNoArgs<reweight>::WithNoArgs(const YAML::Node& config): Model(config)
  {
  }

  template <double(evt::CVUniverse::*reweight)() const>
  events WithNoArgs<reweight>::GetWeight(const evt::CVUniverse& univ) const
  {
    return (univ.*reweight)();
  }
}

//Register some models from PlotUtils that I want to compose at runtime
namespace
{
  static model::WithNoArgs<&evt::CVUniverse::GetMinosEfficiencyWeight>::Registrar reg_MINOS("MINOSEfficiency");
  static model::WithNoArgs<&evt::CVUniverse::GetRPAWeight>::Registrar reg_RPA("RPA");
  static model::WithNoArgs<&evt::CVUniverse::GetLowRecoil2p2hWeight>::Registrar reg_2p2h("2p2h");
}
