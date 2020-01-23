//File: Target1Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET1LEAD_H
#define RECO_TARGET1LEAD_H

//cuts includes
#include "cuts/reco/targets/TwoSectionTarget.h"

//Register Target1Lead for user selection
namespace
{
  static plgn::Registrar<reco::Cut, reco::TwoSectionTarget<true, -1>, std::string&> MainAnalysis_reg("Target1Lead");
}

#endif //RECO_TARGET1LEAD_H
