//File: ODEnergyMax.cpp
//Brief: An UpperLimit on energy in MINERvA's Outer Detector.  I'm hoping
//       to use it to remove events where available energy reconstruction
//       performs poorly.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_HCALENERGYMAX_CPP
#define RECO_HCALENERGYMAX_CPP

//cuts includes
#include "cuts/reco/UpperLimit.h"

namespace
{
  static reco::Cut::Registrar<reco::UpperLimit<MeV, &evt::Universe::GetIDHCALEnergy>> reg_ODMax("HCALEnergyMax");
}

#endif //RECO_HCALENERGYMAX_CPP
