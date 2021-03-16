//File: HasPi0Candidate.h
//Brief: Remove events with neutron candidates that are likely to be from a neutral pion. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_HASPI0CANDIDATE
#define RECO_HASPI0CANDIDATE

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
  class HasPi0Candidate: public Cut
  {
    public:
      HasPi0Candidate(const YAML::Node& config, const std::string& name);
      virtual ~HasPi0Candidate() = default;

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
  };
}

#endif //RECO_HASPI0CANDIDATE
