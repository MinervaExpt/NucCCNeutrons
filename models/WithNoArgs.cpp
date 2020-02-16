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
  //static model::Registrar<model::WithNoArgs<&evt::CVUniverse::GetFluxAndCVWeight>> reg_flux("FluxAndCV"); //This one has default parameters
  static model::Registrar<model::WithNoArgs<&evt::CVUniverse::GetMinosEfficiencyWeight>> reg_MINOS("MINOSEfficiency");
  static model::Registrar<model::WithNoArgs<&evt::CVUniverse::GetGenieWeight>> reg_GENIE("Genie");
  static model::Registrar<model::WithNoArgs<&evt::CVUniverse::GetRPAWeight>> reg_RPA("RPA");
  static model::Registrar<model::WithNoArgs<&evt::CVUniverse::Get2p2hWeight>> reg_2p2h("2p2h");
}
