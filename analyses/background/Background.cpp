//File: Background.cpp
//Brief: A plugin for making plots for events that end up in a background category.
//       Backgrounds are defined by a set of cuts that an event must pass.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//backgrounds includes
#include "analyses/background/Background.h"

//cuts includes
#include "cuts/truth/Cut.h"

namespace bkg
{
  Background::Background(Directory& /*dir*/, cuts_t&& mustPass, const YAML::Node& /*config*/): passes(mustPass)
  {
  }
}
