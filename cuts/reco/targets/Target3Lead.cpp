//File: Target3Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET3LEAD_H
#define RECO_TARGET3LEAD_H

//cuts includes
#include "cuts/reco/targets/ThreeSectionTarget.h"

//Register Target3Lead for user selection
namespace
{
  static plgn::Registrar<reco::Cut, reco::ThreeSectionTarget<82>, std::string&> MainAnalysis_reg("Target3Lead");
}

#endif //RECO_TARGET3LEAD_H
