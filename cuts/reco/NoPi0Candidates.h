//File: NoPi0Candidates.h
//Brief: Remove events with neutron candidates that are likely to be from a neutral pion. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_NOPI0CANDIDATES_H
#define RECO_NOPI0CANDIDATES_H

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//cut includes
#include "cuts/reco/Cut.h"

namespace evt
{
  class Universe;
}

namespace reco
{
  class NoPi0Candidates: public Cut
  {
    public:
      NoPi0Candidates(const YAML::Node& config, const std::string& name);
      virtual ~NoPi0Candidates() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      struct NeutronCand
      {
        MeV edep;
        mm z;
        mm transverse;
        double cosine_to_muon;
      };

      ana::NeutronMultiplicity fCandSelector;

      double fBlobAngleMin; //Minimum blob angle where neutron-rich candidates begin
      double fBlobAngleMax; //Maximum blob angle after which candidates are no longer rich in neutrons
  };
}

#endif //RECO_NOPI0CANDIDATES_H
