//File: Target5Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET5LEAD_H
#define TRUTH_TARGET5LEAD_H

//utility includes
#include "util/Factory.cpp"

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target5Lead for user selection
namespace
{
  static truth::Cut::Registrar<truth::TwoSectionTarget<82>> Target5Lead_reg("Target5Lead");
}

#endif //TRUTH_TARGET5LEAD_H
