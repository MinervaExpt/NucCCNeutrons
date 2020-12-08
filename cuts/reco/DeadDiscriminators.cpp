//File: DeadDiscriminators.cpp
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/DeadDiscriminators.h"

namespace reco
{
  DeadDiscriminators::DeadDiscriminators(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                               fMax(config["max"].as<int>())
  {
  }

  bool DeadDiscriminators::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    return event.GetNDeadDiscriminatorsUpstreamMuon() <= fMax;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::DeadDiscriminators> DeadDiscriminators_reg("DeadDiscriminators");
}
