//File: IsAntineutrino.cpp
//Brief: Requires that an MC event was produced by an antineutrino interaction
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_ISANTINEUTRINO_CPP
#define TRUTH_ISANTINEUTRINO_CPP

//cuts includes
#include "cuts/truth/Helicity.h"

namespace
{
  static truth::Cut::Registrar<truth::Helicity<-14>> Antineutrino_reg("IsAntineutrino");
}


#endif //TRUTH_ISANTINEUTRINO_CPP
