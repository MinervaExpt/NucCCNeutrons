//File: Water.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_WATER_H
#define TRUTH_WATER_H

//utility includes
#include "util/Factory.cpp"

//cuts includes
#include "cuts/truth/targets/OneSectionTarget.h"

//Register Water for user selection
namespace
{
  static plgn::Registrar<truth::Cut, truth::OneSectionTarget<1, 8>> MainAnalysis_reg("Water");
}

#endif //TRUTH_WATER_H
