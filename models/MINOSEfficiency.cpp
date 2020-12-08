//File: MINOSEfficiency.cpp
//Brief: A MINOSEfficiency model modifies the CV's weight to account for a data-MC
//       discrepancy in efficiency to reconstruct a muon in MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//models includes
#include "models/MINOSEfficiency.h"

//evt includes
#include "evt/Universe.h"

namespace model
{
  MINOSEfficiency::MINOSEfficiency(const YAML::Node& config): Model(config)
  {
  }

  events MINOSEfficiency::GetWeight(const evt::Universe& univ) const
  {
    return univ.GetMinosEfficiencyWeight();
  }
}

namespace
{
  model::Model::Registrar<model::MINOSEfficiency> reg_MINOSEfficiency("MINOSEfficiency");
}
