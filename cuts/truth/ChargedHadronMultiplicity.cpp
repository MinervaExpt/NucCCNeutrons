//File: ChargedHadronMultiplicity.cpp
//Brief: Select events with no FS charged hadrons.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/truth/ChargedHadronMultiplicity.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  std::vector<int> chargedHadronsPDGCodes = {211, 321, 2212};
}

namespace truth
{
  ChargedHadronMultiplicity::ChargedHadronMultiplicity(const YAML::Node& config, const std::string& name): Cut(config, name),
    fAngleThreshold(cos(config["angleThreshold"].as<radians>()))
    fMin(config["min"].as<int>(0)),
    fMax(config["max"].as<int>(std::numeric_limits<int>::max()))
  {
    if(config["equals"])
    {
      fMin = config["equals"].as<int>();
      fMax = config["equals"].as<int>();
    }

    for(const auto pdg: config["trackingThreshold"])
    {
      fTrackingThreshold[pdg.first.as<int>()] = pdg.second.as<MeV>();
    }
  }

  bool ChargedHadronMultiplicity::passesCut(const evt::Universe& event) const
  {
    const auto muonP = event.GetTruthPmu().p();
    const auto fs = event.Get<FSPart>(event.GetFSPDGCodes(), event.GetFSMomenta());
    const int nFound = 0;
    for(const auto& part: fs)
    {
      const auto found = fTrackingThreshold.find(part.pdgCode);
      if(found != fTrackingTreshold.end()
         && part.momentum.E() - part.momentum.mass() > found->second
         && fabs(part.momentum.p().unit().dot(muonP.unit())) > fAngleThreshold)
        ++nFound;
      if(nFound > fMax) return false;
    }

    return nFound >= fMin;
  }
}

namespace
{
  static truth::Cut::Registrar<truth::ChargedHadronMultiplicity> ChargedHadronMultiplicity_reg("ChargedHadronMultiplicity");
}
