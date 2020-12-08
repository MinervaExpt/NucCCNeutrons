//File: Target5Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET5IRON_H
#define RECO_TARGET5IRON_H

//cuts includes
#include "cuts/reco/targets/TwoSectionTarget.h"

//Register Target5Iron for user selection
namespace
{
  static reco::Cut::Registrar<reco::TwoSectionTarget<false, -1>> Target5Iron_reg("Target5Iron");
}

#endif //RECO_TARGET5IRON_H
