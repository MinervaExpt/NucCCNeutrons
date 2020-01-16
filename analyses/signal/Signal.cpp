//File: Signal.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/signal/Signal.h"

namespace sig
{
  Signal::Signal(const YAML::Node& /*config*/, util::Directory& /*dir*/, std::vector<evt::CVUniverse*>& /*universes*/)
  {
  }
}
