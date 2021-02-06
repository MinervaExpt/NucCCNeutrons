//File: NeutronMultiplicity.cpp
//Brief: Requires that an MC event has exactly a user-defined number of FS neutrons.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_NEUTRONMULTIPLICITY_CPP
#define TRUTH_NEUTRONMULTIPLICITY_CPP

//cuts includes
#include "cuts/truth/ExactMatch.h"

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

namespace
{
  static truth::Cut::Registrar<truth::UpperLimit<ana::NeutronMultiplicity>> MaxNNeutrons_reg("NeutronMultiplicity");
}

#endif //TRUTH_NEUTRONMULTIPLICITY_CPP
