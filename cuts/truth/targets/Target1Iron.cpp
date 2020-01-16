//File: Target1Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET1IRON_H
#define TRUTH_TARGET1IRON_H

//utility includes
#include "util/Factory.cpp"

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target1Iron for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::TwoSectionTarget<26>> MainAnalysis_reg("Target1Iron");
}

#endif //TRUTH_TARGET1IRON_H
