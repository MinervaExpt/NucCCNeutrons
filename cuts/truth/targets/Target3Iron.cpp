//File: Target3Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET3IRON_H
#define TRUTH_TARGET3IRON_H

//cuts includes
#include "cuts/truth/targets/ThreeSectionTarget.h"

//Register Target3Iron for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::ThreeSectionTarget<26>> MainAnalysis_reg("Target3Carbon");
}

#endif //TRUTH_TARGET3IRON_H
