//File: MaxNumberOfNeutrons.cpp
//Brief: Requires that an MC event has no more than a user-defined maximum number of FS neutrons.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_MAXNUMBEROFNEUTRONS_CPP
#define TRUTH_MAXNUMBEROFNEUTRONS_CPP

//cuts includes
#include "cuts/truth/UpperLimit.h"

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

namespace
{
  static truth::Cut::Registrar<truth::UpperLimit<ana::NeutronMultiplicity>> MaxNNeutrons_reg("MaxNumberOfNeutrons");
}


#endif //TRUTH_MAXNUMBEROFNEUTRONS_CPP
