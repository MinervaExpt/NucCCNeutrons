//File: nTracks.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/nTracks.h"

namespace reco
{
  nTracks::nTracks(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                       fMin(config["min"].as<long int>(-1l)),
                                                                       fMax(config["max"].as<long int>(std::numeric_limits<decltype(fMax)>::max()))
  {
  }

  bool nTracks::passesCut(const evt::CVUniverse& event) const
  {
    return event.GetNTracks() >= fMin && event.GetNTracks() <= fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::nTracks> nTracks_reg("nTracks");
}
