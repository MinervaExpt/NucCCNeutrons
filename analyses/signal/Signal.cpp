//File: Signal.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/signal/Signal.h"

//background includes
#include "analyses/Background.h"

namespace sig
{
  Signal::Signal(const YAML::Node& /*config*/, util::Directory& /*dir*/, std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::CVUniverse*>>& /*universes*/)
  {
  }
}
