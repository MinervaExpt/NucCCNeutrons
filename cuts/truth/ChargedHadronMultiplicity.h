//File: ChargedHadronMultiplicity.h
//Brief: Select events with no charged hadrons in the final state. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_CHARGEDHADRONMULTIPLICITY
#define TRUTH_CHARGEDHADRONMULTIPLICITY

//cut includes
#include "cuts/truth/Cut.h"

//c++ includes
#include <unordered_map>

namespace evt
{
  class Universe;
}

namespace truth
{
  class ChargedHadronMultiplicity: public Cut
  {
    public:
      ChargedHadronMultiplicity(const YAML::Node& config, const std::string& name);
      virtual ~ChargedHadronMultiplicity() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::Universe& event) const override;

    private:
      std::unordered_map<int, MeV> fTrackingThresholds; //Charged hadrons with less energy than this are ignored
      double fAngleThreshold; //Charged hadrons at an angle larger than this are ignored

      int fMin;
      int fMax;

      struct FSPart
      {
        int pdgCode;
        units::LorentzVector<GeV> momentum;
      };
  };
}

#endif //TRUTH_CHARGEDHADRONMULTIPLICITY
