//File: IsNC.cpp
//Brief: Requires that an MC event was produced by a neutral current interaction.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_ISNC_CPP
#define TRUTH_ISNC_CPP

//cuts includes
#include "cuts/truth/Current.h"

namespace
{
  static truth::Cut::Registrar<truth::Current<0>> Neutrino_reg("IsNC");
}


#endif //TRUTH_ISNC_CPP
