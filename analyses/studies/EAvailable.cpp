//File: EAvailable.cpp
//Brief: A EAvailable VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
/*#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"*/
#include "analyses/studies/Resolution.h"
#include "analyses/studies/NeutronMultiplicity.cpp"

//c++ includes
#include <string>

//evt includes
#include "evt/CVUniverse.h"

//utility includes
#include "util/Factory.cpp"

#ifndef ANA_EAVAILABLE_CPP
#define ANA_EAVAILABLE_CPP

namespace ana
{
  //An available energy VARIABLE for the CrossSection<> templates.
  struct EAvailable
  {
    EAvailable(const YAML::Node& config): fMultiplicity(config) {}

    inline std::string name() const { return "E_available"; }

    GeV truth(const evt::CVUniverse& event) const
    {
      return event.GetTruthEAvailable(); 
    }

    GeV reco(const evt::CVUniverse& event) const
    {
      const auto cands = event.Get<Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_calo_edep());
      const auto neutronE = std::accumulate(cands.begin(), cands.end(), 0_MeV,
                                            [&event, this](const MeV sum, const auto& cand)
                                            {
                                              if(this->fMultiplicity.countAsReco(cand, event.GetVtx()) && cand.caloEdep > 10_MeV)
                                                return sum + cand.caloEdep;

                                              return sum;
                                            });

      return event.GetRecoilE() - neutronE;

      //return event.GetRecoilE();

      //return event.GetEAvailable() + event.GetMuonFuzzEnergy() + event.GetODEnergy();
    }

    private:
      struct Candidate
      {
        MeV edep;
        mm z;
        MeV caloEdep;
      };

      NeutronMultiplicity fMultiplicity;
  };
}

namespace
{
  /*static ana::CrossSectionSignal<ana::EAvailable>::Registrar EAvailableSignal_reg("EAvailableSignal");
  static ana::CrossSectionSideband<ana::EAvailable>::Registrar EAvailableSideband_reg("EAvailableSideband");*/
  static ana::Resolution<ana::EAvailable>::Registrar EAvailableResolution_reg("EAvailableResolution");
}

#endif //ANA_EAVAILABLE_CPP
