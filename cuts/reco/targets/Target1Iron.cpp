//File: Target1Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET1IRON_H
#define RECO_TARGET1IRON_H

//cuts includes
#include "cuts/reco/targets/TwoSectionTarget.h"

//Register Target1Iron for user selection
namespace
{
  static reco::Cut::Registrar<reco::TwoSectionTarget<false, -1>> Target1Iron_reg("Target1Iron");
}

#endif //RECO_TARGET1IRON_H
