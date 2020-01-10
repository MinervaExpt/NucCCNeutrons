//File: Target1Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET1LEAD_H
#define TRUTH_TARGET1LEAD_H

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target1Lead for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::TwoSectionTarget<82>> MainAnalysis_reg("Target1Lead");
}

#endif //TRUTH_TARGET1LEAD_H
