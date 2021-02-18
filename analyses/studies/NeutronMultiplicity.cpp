//File: NeutronMultiplicity.cpp
//Brief: A NeutronMultiplicity VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"
#include "analyses/studies/BackgroundsByPionContent.h"
#include "analyses/studies/CandidateMath.h"

//c++ includes
#include <string>
#include <algorithm>

//util includes
#include "util/vector.h"

//evt includes
#include "evt/Universe.h"

#ifndef ANA_NEUTRONMULTIPLICITY_CPP
#define ANA_NEUTRONMULTIPLICITY_CPP

namespace ana
{
  //A muon momentum magnitude VARIABLE for the CrossSection<> templates.
  struct NeutronMultiplicity
  {
    NeutronMultiplicity(const YAML::Node& config): fTruthMinKE(config["truth"]["MinKE"].as<MeV>()),
                                                   fRecoMinEDep(config["reco"]["MinEDep"].as<MeV>()),
                                                   fRecoMaxZDist(config["reco"]["MaxZDist"].as<mm>()),
                                                   fRecoEDepBoxMin(config["reco"]["EDepBoxMin"].as<MeV>()),
                                                   fRecoDistBoxMax(config["reco"]["DistBoxMax"].as<mm>()),
                                                   fMinZCosine(config["reco"]["MinZCosine"].as<double>(0.)),
                                                   fVertexBoxDist(config["reco"]["VertexBoxDist"].as<mm>(0_mm))
    {
    }

    inline std::string name() const { return "Neutron Multiplicity"; }

    struct Candidate
    {
      MeV edep;
      mm z;
      mm transverse;
    };

    struct FSPart
    {
      int PDGCode;
      MeV energy;
      units::LorentzVector<MeV> momentum;
    };

    //Decide whether a neutron candidate/FS particle should be counted.
    //Splitting code up this way lets me reuse this in studies.
    template <class CAND>
    bool countAsReco(const CAND& cand, const units::LorentzVector<mm>& vertex) const
    {
      return (cand.z - vertex.z() < this->fRecoMaxZDist) && (cand.edep > this->fRecoMinEDep) && (DistFromVertex(vertex, cand) < fRecoDistBoxMax || cand.edep > fRecoEDepBoxMin) && (fabs(CosineWrtMuon(vertex, cand)) > fMinZCosine) && (DistFromVertex(vertex, cand) > fVertexBoxDist);
    }

    template <class FS>
    bool countAsTruth(const FS& fs) const
    {
      return (fs.PDGCode == 2112) && ((fs.energy - 939.6_MeV) > this->fTruthMinKE) && (fabs(cos(fs.momentum.p().theta())) > fMinZCosine);
    }

    neutrons truth(const evt::Universe& event) const
    {
      //Count FS neutrons above an energy deposit threshold
      const auto fs = event.Get<FSPart>(event.GetFSPDGCodes(), event.GetFSEnergies(), event.GetFSMomenta());

      return std::count_if(fs.begin(), fs.end(), [this](const auto& fs)
                                                 { return this->countAsTruth(fs);});
    }

    neutrons reco(const evt::Universe& event) const
    {
      //Count candidates close enough to the vertex and with enough energy deposit
      const auto vertex = event.GetVtx();

      const auto cands = event.Get<Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
      return std::count_if(cands.begin(), cands.end(), [&vertex, this](const auto& cand)
                                                       { return this->countAsReco(cand, vertex);});
    }

    private:
      MeV fTruthMinKE; //Minimum energy deposit cut on candidates
      MeV fRecoMinEDep;
      mm fRecoMaxZDist; //Minimum z distance from vertex for candidates

      //Reinteraction cut box
      MeV fRecoEDepBoxMin;
      mm fRecoDistBoxMax;

      double fMinZCosine; //Minimum angle w.r.t. the muon

      //Vertex box: no candidates allowed inside
      mm fVertexBoxDist;
  };
}

namespace
{
  static ana::CrossSectionSignal<ana::NeutronMultiplicity>::Registrar NeutronMultiplicitySignal_reg("NeutronMultiplicitySignal");
  static ana::CrossSectionSideband<ana::NeutronMultiplicity>::Registrar NeutronMultiplicitySideband_reg("NeutronMultiplicitySideband");
  static ana::BackgroundsByPionContent<ana::NeutronMultiplicity>::Registrar NeutronMultiplicityPionContent_reg("NeutronMultiplicityByPionContent");
}

#endif //ANA_NEUTRONMULTIPLICITY_CPP
