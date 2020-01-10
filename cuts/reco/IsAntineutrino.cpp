//File: IsAntineutrino.cpp
//Brief: Requires that an event was reconstructed as an antineutrino interaction
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_ISANTINEUTRINO_CPP
#define RECO_ISANTINEUTRINO_CPP

//cuts includes
#include "cuts/reco/Helicity.h"

namespace
{
  static plgn::Registrar<reco::Cut, reco::Helicity<2>> MainAnalysis_reg("IsAntineutrino");
}


#endif //RECO_ISANTINEUTRINO_CPP
