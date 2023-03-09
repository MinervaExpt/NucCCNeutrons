//File: NeutronDetectionWithBackgrounds.cpp
//Brief: Reproduces the candidates per event plot from Miranda and Rik's LE neutron counting paper.  Unlike the NeutronDetection Study,
//       this Study also predicts the background rate so that I could do a sideband constraint.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/NeutronDetectionWithBackgrounds.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  std::vector<util::NamedCategory<int>> pdgCategories = {util::NamedCategory<int>{{2112}, "Direct Neutrons"}, //"GENIE Neutrons"}, 
                                                         util::NamedCategory<int>{{std::numeric_limits<int>::max()}, "Reinteraction Neutrons"}, //"GEANT Neutrons"},
                                                         util::NamedCategory<int>{{-211, 211, 2212, 111,22}, "EM and Hadrons"},
                                                         util::NamedCategory<int>{{-13, 13}, "Muon"}
                                                        };
}

namespace ana
{
  NeutronDetectionWithBackgrounds::NeutronDetectionWithBackgrounds(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                                                   std::vector<background_t>& backgrounds,
                                                                   std::map<std::string, std::vector<evt::Universe*>>& univs):
                                                                  Study(config, dir, std::move(mustPass), backgrounds, univs), fCuts(config["variable"]),
                                                                  fSignalEDeps(pdgCategories, dir, "", "energy deposit",
                                                                               config["binning"]["edep"].as<std::vector<double>>(), univs),
                                                                  fBackgroundEDeps(backgrounds, dir, "Background", "energy deposit", pdgCategories, dir,
                                                                                   config["binning"]["edep"].as<std::vector<double>>(), univs)
  {
    fDataEDeps = dir.make<EDepHist>("Data", "Data;Reco", config["binning"]["edep"].as<std::vector<double>>(), univs);
    fNMCEntries = dir.make<PlotUtils::HistWrapper<evt::Universe>>("NMCEntries", "Number of Signal Selected Entries", 1, 0, 1, univs);
    fNDataEntries = dir.make<PlotUtils::HistWrapper<evt::Universe>>("NDataEntries", "Number of Selected Entries", 1, 0, 1, univs);
  }

  void NeutronDetectionWithBackgrounds::data(const evt::Universe& event, const events weight)
  {
    fNDataEntries->FillUniverse(event, 0.5, weight.in<events>());

    const auto cands = event.Get<NeutronCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(), event.Getblob_nViews());
    const auto vertex = event.GetVtx();

    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex)) fDataEDeps->Fill(&event, cand.edep, neutrons(weight.in<events>()));
    }
  }

  void NeutronDetectionWithBackgrounds::mcSignal(const evt::Universe& event, const events weight)
  {
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(), event.Getblob_nViews(), event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z(), event.GetFSMomenta());
    const auto vertex = event.GetVtx();

    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex))
      {
        const int pdg = encodeFSPDG(cand, fs);
        fSignalEDeps[pdg].Fill(&event, cand.edep, neutrons(weight.in<events>()));
      }
    }
  }

  void NeutronDetectionWithBackgrounds::mcBackground(const evt::Universe& event, const background_t& background, const events weight)
  {
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(), event.Getblob_nViews(), event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z(), event.GetFSMomenta());
    const auto vertex = event.GetVtx();
  
    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex))
      {
        const int pdg = encodeFSPDG(cand, fs);
        fBackgroundEDeps[background][pdg].Fill(&event, cand.edep, neutrons(weight.in<events>()));
      }
    }
  }

  void NeutronDetectionWithBackgrounds::afterAllFiles(const events passedSelection)
  {
    fSignalEDeps.visit([](auto& hist) { hist.SyncCVHistos(); });
    fDataEDeps->SyncCVHistos();
    fBackgroundEDeps.visit([](auto& category)
                           {
                             category.visit([](auto& hist) { hist.SyncCVHistos(); });
                           });

    fNMCEntries->SyncCVHistos();
    fNDataEntries->SyncCVHistos();
  }

  int NeutronDetectionWithBackgrounds::encodeFSPDG(const MCCandidate& cand, const std::vector<FSPart>& fs) const
  {
    int pdg = -1; //-1 represents "Other".  Thsi happens when I couldn't find a parent PDG code.
                      //I think that means either overlay or cross-talk.
    if(cand.FS_index >= 0)
    {
      pdg = fs[cand.FS_index].PDGCode;
      //Check for "GEANT neutron"s: FS particles that weren't neutrons but produced neutrons that I detected.
      if(pdg != 2112 && cand.dist_to_edep_as_neutron > 0_mm) pdg = std::numeric_limits<int>::max();
    }

    return pdg;
  }
}

namespace
{
  static ana::Study::Registrar<ana::NeutronDetectionWithBackgrounds> NeutronDetectionWithBackgrounds_reg("NeutronDetectionWithBackgrounds");
}
