//File: NeutronMultiplicity.cpp
//Brief: Requires that an MC event has exactly a user-defined number of FS neutrons.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_NEUTRONMULTIPLICITY_CPP
#define RECO_NEUTRONMULTIPLICITY_CPP

//cuts includes
#include "cuts/reco/Range.h"

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

namespace
{
  static reco::Cut::Registrar<reco::Range<ana::NeutronMultiplicity>> MaxNNeutrons_reg("NeutronMultiplicity");
}

#endif //RECO_NEUTRONMULTIPLICITY_CPP
