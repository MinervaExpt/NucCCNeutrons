//File: Target5Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET5LEAD_H
#define TRUTH_TARGET5LEAD_H

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target5Lead for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::TwoSectionTarget<82>> MainAnalysis_reg("Target5Lead");
}

#endif //TRUTH_TARGET5LEAD_H
