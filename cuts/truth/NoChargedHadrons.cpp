//File: NoChargedHadrons.cpp
//Brief: Select events with no FS charged hadrons.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/NoChargedHadrons.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  std::vector<int> chargedHadronsPDGCodes = {211, 321, 2212};
}

namespace truth
{
  NoChargedHadrons::NoChargedHadrons(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                                         fTrackingThreshold(config["trackingThreshold"].as<MeV>(100_MeV))
  {
  }

  bool NoChargedHadrons::passesCut(const evt::Universe& event) const
  {
    const auto fs = event.Get<FSPart>(event.GetFSPDGCodes(), event.GetFSEnergies());
    for(const auto& part: fs)
    {
      if(part.energy > fTrackingThreshold
         && std::find(chargedHadronsPDGCodes.begin(), chargedHadronsPDGCodes.end(), abs(part.pdgCode)) != chargedHadronsPDGCodes.end())
        return false;
    }

    return true;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::NoChargedHadrons> NoChargedHadrons_reg("NoChargedHadrons");
}
