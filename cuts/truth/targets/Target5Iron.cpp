//File: Target5Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET5IRON_H
#define TRUTH_TARGET5IRON_H

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//utility includes
#include "util/Factory.cpp"

//Register Target5Iron for user selection
namespace
{
  static truth::Cut::Registrar<truth::TwoSectionTarget<26>> Target5Iron_reg("Target5Iron");
}

#endif //TRUTH_TARGET5IRON_H
