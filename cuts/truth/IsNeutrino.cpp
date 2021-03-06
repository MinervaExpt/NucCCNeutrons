//File: IsNeutrino.cpp
//Brief: Requires that an MC event was produced by a neutrino interaction
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_ISNEUTRINO_CPP
#define TRUTH_ISNEUTRINO_CPP

//cuts includes
#include "cuts/truth/Helicity.h"

namespace
{
  static truth::Cut::Registrar<truth::Helicity<14>> Neutrino_reg("IsNeutrino");
}


#endif //TRUTH_ISNEUTRINO_CPP
