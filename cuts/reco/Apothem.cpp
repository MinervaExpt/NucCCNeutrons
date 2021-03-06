//File: Apothem.cpp
//Brief: Select events within some fiducial apothem.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/Apothem.h"

using namespace units;

namespace reco
{
  Apothem::Apothem(const YAML::Node& config, const std::string& name): Cut(config, name), fApothem(config["apothem"].as<mm>())
  {
  }

  bool Apothem::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const auto vertex = event.GetVtx();
    return (fabs(vertex.y()) < mm(fSlope*fabs(vertex.x().in<mm>()) + 2.*fApothem.in<mm>()/sqrt(3.)))
           && (fabs(vertex.x()) < fApothem);
  }
}

namespace
{
  static reco::Cut::Registrar<reco::Apothem> Apothem_reg("Apothem");
}
