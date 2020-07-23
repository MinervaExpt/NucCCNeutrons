//File: NeutronPurity.cpp
//Brief: This Study quantifies how well I differentiate GENIE-neutron-induced candidates from other candidates.
//       It's a good place to plot variables I'm considering adding to my candidate selection.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/NeutronPurity.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  //TODO: Standardize these.  They're also used in the NeutronDetection Study.
  std::vector<util::NamedCategory<int>> pdgCategories = {util::NamedCategory<int>{{2112}, "Direct Neutrons"}, //"GENIE Neutrons"}, 
                                                         util::NamedCategory<int>{{std::numeric_limits<int>::max()}, "Reinteraction Neutrons"}, //"GEANT Neutrons"},
                                                         util::NamedCategory<int>{{111}, "#pi^{0}"},
                                                         util::NamedCategory<int>{{22}, "#gamma"},
                                                         util::NamedCategory<int>{{-211, 211, 2212}, "Charged Hadrons"},
                                                         util::NamedCategory<int>{{-13, 13}, "Muon"}
                                                        };
}

namespace ana
{
  NeutronPurity::NeutronPurity(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fCuts(config["variable"]),
                                                                                                   fPDGToEDepVersusNClusters(pdgCategories, dir, "", "Number of Clusters;Visisble Candidate Energy;Number of candidates",
                                                                                                                             config["binning"]["nClusters"].as<std::vector<double>>(),
                                                                                                                             config["binning"]["edep"].as<std::vector<double>>(),
                                                                                                                             univs)
  {
  }

  void NeutronPurity::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    //Cache weight for each universe
    const neutrons weightPerNeutron = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_calo_edep(),
                                              event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(),
                                              event.Getblob_earliest_time(), event.Getblob_n_clusters(),
                                              event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z());
    const auto vertex = event.GetVtx();

    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex))
      {
        int pdg = -1; //-1 represents "Other".  This happens when I couldn't find a parent PDG code.
                      //I think that means either overlay or cross-talk.
        if(cand.FS_index >= 0)
        {
          pdg = fs[cand.FS_index].PDGCode;
          //Check for "GEANT neutron"s: FS particles that weren't neutrons but produced neutrons that I detected.
          if(pdg != 2112 && cand.dist_to_edep_as_neutron > 0_mm) pdg = std::numeric_limits<int>::max();
        }

        fPDGToEDepVersusNClusters[pdg].Fill(&event, Clusters(cand.nClusters), cand.caloEdep, weightPerNeutron);
      }
    }
  }

  void NeutronPurity::afterAllFiles(const events passedSelection)
  {
    fPDGToEDepVersusNClusters.visit([passedSelection](auto& hist)
                                    {
                                      //hist.hist->Scale(1./passedSelection.in<events>());
                                      hist.SyncCVHistos();
                                    });
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::NeutronPurity> NeutronPurity_reg("NeutronPurity");
}
