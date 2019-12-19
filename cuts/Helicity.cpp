//File: Helicity.cpp
//Brief: A Cut on whether this is a neutrino or an anti-neutrino.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Cuts includes
#include "Cuts/Helicity.h"

namespace apo
{
  Helicity::Helicity(const YAML::Node& config): fHelicity(config.as<int>("Helicity"))
  {
  }

  bool Helicity::doCut(const SchemaView& event)
  {
    return event.helicity == fHelicity;
  }
}

//Register Helicity with Factory<Cut>
namespace
{
  static plgn::Registrar<apo::Cut, apo::Helicity> Helicity_reg("Helicity");
}
