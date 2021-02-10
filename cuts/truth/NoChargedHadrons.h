//File: NoChargedHadrons.h
//Brief: Select events with no charged hadrons in the final state. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_NOCHARGEDHADRONS_H
#define TRUTH_NOCHARGEDHADRONS_H

//cut includes
#include "cuts/truth/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class Universe;
}

namespace truth
{
  class NoChargedHadrons: public Cut
  {
    public:
      NoChargedHadrons(const YAML::Node& config, const std::string& name);
      virtual ~NoChargedHadrons() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::Universe& event) const override;

    private:
      MeV fTrackingThreshold;

      struct FSPart
      {
        int pdgCode;
        GeV energy;
      };
  };
}

#endif //TRUTH_NOCHARGEDHADRONS_H
