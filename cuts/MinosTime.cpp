//File: MinosTime.cpp
//Brief: A Cut on whether this is a neutrino or an anti-neutrino.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Cuts includes
#include "Cuts/MinosTime.h"

namespace apo
{
  MinosTime::MinosTime(const YAML::Node& config): fMinTime(config.as<double>("MinTime")),
                                                  fMaxTime(config.as<double>("MaxTime"))
  {
  }

  bool MinosTime::doCut(const SchemaView& event)
  {
    return event.minosDeltaT > fMinTime && event.minosDeltaT < fMaxTime;
  }
}

//Register MinosTime with Factory<Cut>
namespace
{
  static plgn::Registrar<apo::Cut, apo::MinosTime> MinosTime_reg("MinosTime");
}
