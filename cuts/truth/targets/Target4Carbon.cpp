//File: Target4Carbon.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET4CARBON_H
#define TRUTH_TARGET4CARBON_H

//utility includes
#include "util/Factory.cpp"

//cuts includes
#include "cuts/truth/targets/OneSectionTarget.h"

//Register Target4Carbon for user selection
namespace
{
  static truth::Cut::Registrar<truth::OneSectionTarget<6>> Target4Carbon_reg("Target4Carbon");
}

#endif //TRUTH_TARGET4CARBON_H
