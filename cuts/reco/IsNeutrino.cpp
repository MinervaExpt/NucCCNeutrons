//File: IsNeutrino.cpp
//Brief: Requires that an event was reconstructed as a neutrino interaction
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_ISNEUTRINO_CPP
#define RECO_ISNEUTRINO_CPP

//cuts includes
#include "cuts/reco/Helicity.h"

namespace
{
  static plgn::Registrar<reco::Cut, reco::Helicity<1>, std::string&> MainAnalysis_reg("IsNeutrino");
}


#endif //RECO_ISNEUTRINO_CPP
