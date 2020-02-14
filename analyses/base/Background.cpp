//File: Background.cpp
//Brief: A plugin for making plots for events that end up in a background category.
//       Backgrounds are defined by a set of cuts that an event must pass.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//backgrounds includes
#include "analyses/base/Background.h"

//cuts includes
#include "cuts/truth/Cut.h"

//util includes
#include "util/Factory.cpp"

namespace bkg
{
  Background::Background(const std::string& nodeName, const YAML::Node& config): passes(plgn::loadPlugins<truth::Cut>(config)),
                                                                                 fName(nodeName)
  {
  }
}
