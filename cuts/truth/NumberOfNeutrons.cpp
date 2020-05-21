//File: IsNeutrino.cpp
//Brief: Requires that an MC event was produced by a neutrino interaction
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_NUMBEROFNEUTRONS_CPP
#define TRUTH_NUMBEROFNEUTRONS_CPP

//cuts includes
#include "cuts/truth/UpperLimit.h"

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

namespace
{
  static truth::Cut::Registrar<truth::UpperLimit<ana::NeutronMultiplicity>> NNeutrons_reg("NumberOfNeutrons");
}


#endif //TRUTH_NUMBEROFNEUTRONS_CPP
