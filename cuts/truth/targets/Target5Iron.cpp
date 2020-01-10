//File: Target5Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET5IRON_H
#define TRUTH_TARGET5IRON_H

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target5Iron for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::TwoSectionTarget<26>> MainAnalysis_reg("Target5Iron");
}

#endif //TRUTH_TARGET5IRON_H
