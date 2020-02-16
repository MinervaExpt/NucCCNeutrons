//File: Target2Lead.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET2LEAD_H
#define TRUTH_TARGET2LEAD_H

//utility includes
#include "util/Factory.cpp"

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target2Lead for user selection
namespace
{
  static truth::Cut::Registrar<truth::TwoSectionTarget<82>> Target2Lead_reg("Target2Lead");
}

#endif //TRUTH_TARGET2LEAD_H
