//File: GEANTUncertainties.cpp
//Brief: A GEANTUncertainties model modifies the CV's weight to account for
//       uncertainties in parameters of our particle transport simulation.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//models includes
#include "models/GEANTUncertainties.h"

//evt includes
#include "evt/Universe.h"

namespace model
{
  GEANTUncertainties::GEANTUncertainties(const YAML::Node& config): Model(config)
  {
    PlotUtils::MinervaUniverse::SetMHRWeightNeutronCVReweight(config["UseNeutronReweight"].as<bool>());
    PlotUtils::MinervaUniverse::SetMHRWeightElastics(config["ReweightElastics"].as<bool>(true));
    //TODO: Connect PlotUtils::MinervaUniverse::SetReadoutVolume() to how Fiducials were set up.
    //      This means I need a different set of weights for each fiducial though.  That's a limitation
    //      of the MAT it seems.
    auto fileName = config["ReweightFile"];
    if(fileName)
    {
       PlotUtils::MinervaUniverse::SetMHRWeightFilename(config["ProcessName"].as<std::string>(), fileName.as<std::string>());
    }
  }

  events GEANTUncertainties::GetWeight(const evt::Universe& univ) const
  {
    return univ.GetGeantHadronWeight();
  }
}

namespace
{
  model::Model::Registrar<model::GEANTUncertainties> reg_flux("GEANT");
}
