//File: Target3Carbon.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET3CARBON_H
#define RECO_TARGET3CARBON_H

//cuts includes
#include "cuts/reco/targets/ThreeSectionTarget.h"

//Register Target3Carbon for user selection
namespace
{
  static plgn::Registrar<reco::Cut, reco::ThreeSectionTarget<6>> MainAnalysis_reg("Target3Carbon");
}

#endif //RECO_TARGET3CARBON_H
