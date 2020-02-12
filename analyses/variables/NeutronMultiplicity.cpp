//File: NeutronMultiplicity.cpp
//Brief: A NeutronMultiplicity VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <string>
#include <algorithm>

//analyses includes
//That's right, I'm covering up a base class function.  Suppress the warnings from it because "I know what I'm doing".
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "analyses/signal/CrossSection.h"
#include "analyses/sideband/CrossSection.h"
#pragma GCC diagnostic pop

//evt includes
#include "evt/CVUniverse.h"

//cuts includes
#include "cuts/reco/Cut.h"
#include "cuts/truth/Cut.h"

//util includes
#include "util/Factory.cpp"

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
      mm transverse;
    };

    struct FSPart
    {
      int PDGCode;
      MeV energy;
    };

    neutrons truth(const evt::CVUniverse& event)
    {
      //Count FS neutrons above an energy deposit threshold
      const auto fs = event.Get<FSPart>(event.GetFSPDG_code(), event.GetFSenergy());
      return std::count_if(fs.begin(), fs.end(), [this](const auto& fs)
                                                 { return fs.PDGCode == 2112 && (fs.energy - 939.6_MeV) > this->fTruthMinEDep;});
    }

    neutrons reco(const evt::CVUniverse& event)
    {
      //Count candidates close enough to the vertex and with enough energy deposit
      const auto vertex = event.GetVtx();

      const auto cands = event.Get<Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
      return std::count_if(cands.begin(), cands.end(), [&vertex, this](const auto& cand)
                                                       { return cand.z - vertex.z() < this->fRecoMaxZDist && cand.edep > this->fRecoMinEDep;});
    }

    private:
      MeV fTruthMinEDep; //Minimum energy deposit cut on candidates
      MeV fRecoMinEDep;
      mm fRecoMaxZDist; //Minimum z distance from vertex for candidates
  };
}

namespace
{
  static plgn::Registrar<sig::Signal, sig::CrossSection<ana::NeutronMultiplicity>, util::Directory&,
                         std::vector<typename sig::Signal::background_t>&,
                         std::map<std::string, std::vector<evt::CVUniverse*>>&> NeutronMultiplicitySignal_reg("NeutronMultiplicity");
  static plgn::Registrar<side::Sideband, side::CrossSection<ana::NeutronMultiplicity>, util::Directory&,
                         typename side::Sideband::cuts_t&&, std::vector<typename side::Sideband::background_t>&,
                         std::map<std::string, std::vector<evt::CVUniverse*>>&> NeutronMultiplicitySideband_reg("NeutronMultiplicity");
}
