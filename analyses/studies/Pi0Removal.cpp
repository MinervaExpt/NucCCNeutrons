//File: Pi0Removal.cpp
//Brief: This Study quantifies how well I differentiate GENIE-neutron-induced candidates from other candidates.
//       It's a good place to plot variables I'm considering adding to my candidate selection.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/studies/Pi0Removal.h"
#include "analyses/studies/CandidateMath.h"

//util includes
#include "util/Factory.cpp"

//evt includes
#include "evt/EventID.h"
#include "evt/arachne.h"

//c++ includes
#include <cmath>
#include <unordered_map>

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
  Pi0Removal::Pi0Removal(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                 fCuts(config["variable"]),
                fPDGToBestInvMass(pdgCategories, dir, "BestInvariantMass", "M;candidates", 100, 0, 300, univs),
                fPDGToCosineToMuon(pdgCategories, dir, "CosineToMuon", "Candidate Fit Direction;candidates", 60, -1, 1, univs)
  {
  }

  void Pi0Removal::mcSignal(const evt::Universe& event, const events weight)
  {
    //Cache weight for each universe
    const neutrons weightPerNeutron = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex(),
                                              event.Getblob_earliest_time(), event.Getblob_FS_index(),
                                              event.Getblob_geant_dist_to_edep_as_neutron(),
                                              event.Getblob_nViews(), event.Getblob_direction_difference(),
                                              event.Getblob_3D_start_x(), event.Getblob_3D_start_y());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetFSMomenta());
    const auto vertex = event.GetVtx();

    std::vector<MCCandidate> fitCands;
    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex) && cand.cosine_to_muon > -2) fitCands.push_back(cand);
    }

    for(auto whichCand = fitCands.begin(); whichCand != fitCands.end(); ++whichCand)
    {
      const auto& cand = *whichCand;
      int pdg = -1; //-1 represents "Other".  This happens when I couldn't find a parent PDG code.
                    //I think that means either overlay or cross-talk.
      if(cand.FS_index >= 0)
      {
        pdg = fs[cand.FS_index].PDGCode;
        //Check for "GEANT neutron"s: FS particles that weren't neutrons but produced neutrons that I detected.
        if(pdg != 2112 && cand.dist_to_edep_as_neutron > 0_mm) pdg = std::numeric_limits<int>::max();
      }

      //Find closest invariant mass to the pi0 mass among all 3D candidates
      const auto pi0Mass = 134.967_MeV;
      const auto closest = std::min_element(whichCand+1, fitCands.end(),
                                            [pi0Mass, &vertex, &cand](const auto& lhs, const auto& rhs)
                                            {
                                              return fabs(ana::InvariantMass(vertex, cand, lhs) - pi0Mass)
                                                     < fabs(ana::InvariantMass(vertex, cand, rhs) - pi0Mass);
                                            });
      if(closest != fitCands.end()) fPDGToBestInvMass[pdg].Fill(&event, ana::InvariantMass(vertex, cand, *closest), weightPerNeutron);

      //Plot cosine to muon
      fPDGToCosineToMuon[pdg].FillUniverse(&event, cand.cosine_to_muon, weightPerNeutron.in<neutrons>()); 
    }
  }

  void Pi0Removal::afterAllFiles(const events /*passedSelection*/)
  {
    fPDGToBestInvMass.visit([](auto& hist) { hist.SyncCVHistos(); });
    fPDGToCosineToMuon.visit([](auto& hist) { hist.SyncCVHistos(); });
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::Pi0Removal> Pi0Removal_reg("Pi0Removal");
}
