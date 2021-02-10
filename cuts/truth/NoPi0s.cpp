//File: NoPi0s.cpp
//Brief: Select events with no FS neutral pions.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/NoPi0s.h"

//util includes
#include "util/Factory.cpp"

namespace truth
{
  NoPi0s::NoPi0s(const YAML::Node& config, const std::string& name): Cut(config, name)
  {
  }

  bool NoPi0s::passesCut(const evt::Universe& event) const
  {
    const auto fs = event.Get<FSPart>(event.GetFSPDGCodes(), event.GetFSEnergies());
    for(const auto& part: fs)
    {
      if(part.pdgCode == 111) return false;
    }

    return true;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::NoPi0s> NoPi0s_reg("NoPi0s");
}
