//File: Target2Iron.h
//Brief: An (x, y) cut to isolate the iron in target 1.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TARGET2IRON_H
#define TRUTH_TARGET2IRON_H

//utility includes
#include "util/Factory.cpp"

//cuts includes
#include "cuts/truth/targets/TwoSectionTarget.h"

//Register Target2Iron for user selection
namespace
{
  static truth::Cut::Registrar<truth::TwoSectionTarget<26>> Target2Iron_reg("Target2Iron");
}

#endif //TRUTH_TARGET2IRON_H
