//File: IsCC.cpp
//Brief: Requires that an MC event was produced by a charged current interaction.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_ISCC_CPP
#define TRUTH_ISCC_CPP

//cuts includes
#include "cuts/truth/Current.h"

namespace
{
  static truth::Cut::Registrar<truth::Current<1>> Neutrino_reg("IsCC");
}


#endif //TRUTH_ISCC_CPP
