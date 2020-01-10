//File: Sideband.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//sideband includes
#include "analyses/sideband/Sideband.h"

//evt includes
#include "evt/CVUniverse.h"

//bkg includes
#include "analyses/backgrounds/Background.h"

//cuts includes
#include "cuts/reco/Cut.h"

namespace side
{
  Sideband::Sideband(util::Directory& /*dir*/, cuts_t&& mustPass,
                     const backgrounds_t& /*backgrounds*/, const YAML::Node& /*config*/): passes(mustPass)
  {
  }
}
