//File: Target2Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET2IRON_H
#define RECO_TARGET2IRON_H

//cuts includes
#include "cuts/reco/targets/TwoSectionTarget.h"

//Register Target2Iron for user selection
namespace
{
  static reco::Cut::Registrar<reco::TwoSectionTarget<false, 1>> Target2Iron_reg("Target2Iron");
}

#endif //RECO_TARGET2IRON_H
