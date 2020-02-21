//File: GENIEUncertainties.cpp
//Brief: A GENIEUncertainties model modifies the CV's weight to account for
//       uncertainties in parameters of our neutrino event generator.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//models includes
#include "models/GENIEUncertainties.h"

//evt includes
#include "evt/CVUniverse.h"

namespace model
{
  GENIEUncertainties::GENIEUncertainties(const YAML::Node& config): Model(config)
  {
    PlotUtils::DefaultCVUniverse::SetNonResPiReweight(config["UseNonResonantPion"].as<bool>());
  }

  events GENIEUncertainties::GetWeight(const evt::CVUniverse& univ) const
  {
    return univ.GetGenieWeight();
  }
}

namespace
{
  model::Model::Registrar<model::GENIEUncertainties> reg_flux("GENIE");
}
