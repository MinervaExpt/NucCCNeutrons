//File: NeutronMultiplicity.cpp
//Brief: A NeutronMultiplicity VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"

//c++ includes
#include <string>
#include <algorithm>

//util includes
#include "util/vector.h"

//evt includes
#include "evt/CVUniverse.h"

#ifndef ANA_NEUTRONMULTIPLICITY_CPP
#define ANA_NEUTRONMULTIPLICITY_CPP

namespace ana
{
  //A muon momentum magnitude VARIABLE for the CrossSection<> templates.
  struct NeutronMultiplicity
  {
    NeutronMultiplicity(const YAML::Node& config): fTruthMinEDep(config["truth"]["MinEDep"].as<MeV>()),
                                                   fRecoMinEDep(config["reco"]["MinEDep"].as<MeV>()),
                                                   fRecoMaxZDist(config["reco"]["MaxZDist"].as<mm>())
    {
    }

    inline std::string name() const { return "Neutron Multiplicity"; }

    struct Candidate
    {
      MeV edep;
      mm z;
    };

    struct FSPart
    {
      int PDGCode;
      MeV energy;
    };

    //Decide whether a neutron candidate/FS particle should be counted.
    //Splitting code up this way lets me reuse this in studies.
    template <class CAND>
    bool countAsReco(const CAND& cand, const units::LorentzVector<mm>& vertex) const
    {
      return cand.z - vertex.z() < this->fRecoMaxZDist && cand.edep > this->fRecoMinEDep;
    }

    template <class FS>
    bool countAsTruth(const FS& fs) const
    {
      return fs.PDGCode == 2112 && (fs.energy - 939.6_MeV) > this->fTruthMinEDep;
    }

    neutrons truth(const evt::CVUniverse& event)
    {
      //Count FS neutrons above an energy deposit threshold
      const auto fs = event.Get<FSPart>(event.GetFSPDG_code(), event.GetFSenergy());
      return std::count_if(fs.begin(), fs.end(), [this](const auto& fs)
                                                 { return this->countAsTruth(fs);});
    }

    neutrons reco(const evt::CVUniverse& event)
    {
      //Count candidates close enough to the vertex and with enough energy deposit
      const auto vertex = event.GetVtx();

      const auto cands = event.Get<Candidate>(event.Getblob_edep(), event.Getblob_zPos());
      return std::count_if(cands.begin(), cands.end(), [&vertex, this](const auto& cand)
                                                       { return this->countAsReco(cand, vertex);});
    }

    private:
      MeV fTruthMinEDep; //Minimum energy deposit cut on candidates
      MeV fRecoMinEDep;
      mm fRecoMaxZDist; //Minimum z distance from vertex for candidates
  };
}

namespace
{
  static ana::CrossSectionSignal<ana::NeutronMultiplicity>::Registrar NeutronMultiplicitySignal_reg("NeutronMultiplicitySignal");
  static ana::CrossSectionSideband<ana::NeutronMultiplicity>::Registrar NeutronMultiplicitySideband_reg("NeutronMultiplicitySideband");
}

#endif //ANA_NEUTRONMULTIPLICITY_CPP
