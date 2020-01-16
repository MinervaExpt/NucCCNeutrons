//File: Target5Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET5LEAD_H
#define RECO_TARGET5LEAD_H

//cuts includes
#include "cuts/reco/targets/TwoSectionTarget.h"

//Register Target5Lead for user selection
namespace
{
  static plgn::Registrar<reco::Cut, reco::TwoSectionTarget<true, -1>> MainAnalysis_reg("Target5Lead");
}

#endif //RECO_TARGET5LEAD_H
