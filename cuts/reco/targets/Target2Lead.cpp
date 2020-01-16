//File: Target2Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TARGET2LEAD_H
#define RECO_TARGET2LEAD_H

//cuts includes
#include "cuts/reco/targets/TwoSectionTarget.h"

//Register Target2Lead for user selection
namespace
{
  static plgn::Registrar<reco::Cut, reco::TwoSectionTarget<true, 1>> MainAnalysis_reg("Target2Lead");
}

#endif //RECO_TARGET2LEAD_H
