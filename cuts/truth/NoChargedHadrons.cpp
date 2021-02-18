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
                                                                                         fTrackingThreshold(config["trackingThreshold"].as<MeV>(100_MeV)),
                                                                                         fAngleThreshold(cos(config["angleThreshold"].as<radians>()))
  {
  }

  bool NoChargedHadrons::passesCut(const evt::Universe& event) const
  {
    const auto muonP = event.GetTruthPmu().p();
    const auto fs = event.Get<FSPart>(event.GetFSPDGCodes(), event.GetFSMomenta());
    for(const auto& part: fs)
    {
      if(part.momentum.E() - part.momentum.mass() > fTrackingThreshold
         && fabs(part.momentum.p().unit().dot(muonP.unit())) > fAngleThreshold
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
