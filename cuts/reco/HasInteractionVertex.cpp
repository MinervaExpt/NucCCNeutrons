//File: HasInteractionVertex.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/HasInteractionVertex.h"

namespace reco
{
  HasInteractionVertex::HasInteractionVertex(const YAML::Node& config, const std::string& name): Cut(config, name)
  {
  }

  bool HasInteractionVertex::passesCut(const evt::CVUniverse& event) const
  {
    return event.hasInteractionVertex();
  }
}

namespace
{
  static reco::Cut::Registrar<reco::HasInteractionVertex> HasInteractionVertex_reg("HasInteractionVertex");
}
