//File: Sideband.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//sideband includes
#include "analyses/sideband/Sideband.h"

//evt includes
#include "evt/CVUniverse.h"

//bkg includes
#include "analyses/Background.h"

//cuts includes
#include "cuts/reco/Cut.h"

namespace side
{
  Sideband::Sideband(const YAML::Node& /*config*/, util::Directory& /*dir*/,
                     cuts_t&& mustPass, const std::vector<background_t>& /*backgrounds*/,
                     std::vector<evt::CVUniverse*>& /*universes*/): passes(std::move(mustPass))
  {
  }
}
