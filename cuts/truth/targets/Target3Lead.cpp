//File: Target3Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET3LEAD_H
#define TRUTH_TARGET3LEAD_H

//cuts includes
#include "cuts/truth/targets/ThreeSectionTarget.h"

//Register Target3Lead for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::ThreeSectionTarget<82>> MainAnalysis_reg("Target3Carbon");
}

#endif //TRUTH_TARGET3LEAD_H
