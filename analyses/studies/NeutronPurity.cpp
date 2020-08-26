//File: NeutronPurity.cpp
//Brief: This Study quantifies how well I differentiate GENIE-neutron-induced candidates from other candidates.
//       It's a good place to plot variables I'm considering adding to my candidate selection.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/studies/NeutronPurity.h"

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
  mm NeutronPurity::distToVertex(const MCCandidate& cand, const units::LorentzVector<mm>& vertex)
  {
    using namespace units;
    return sqrt(pow<2>(cand.transverse) + pow<2>(cand.z - vertex.z()));
  }

  NeutronPurity::NeutronPurity(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fCuts(config["variable"]),
                                                                                                   fPDGToEDepVersusNClusters(pdgCategories, dir, "EDepVersusNClusters", "Number of Clusters;Visisble Candidate Energy;Number of candidates",
                                                                                                                             config["binning"]["nClusters"].as<std::vector<double>>(),
                                                                                                                             config["binning"]["edep"].as<std::vector<double>>(),
                                                                                                                             univs),
                fPDGToEDepVersusNDigits(pdgCategories, dir, "EDepVersusNDigits", "Number of Digits;Visisble Candidate Energy;Number of candidates",

                                          config["binning"]["nDigits"].as<std::vector<double>>(),

                                          config["binning"]["edep"].as<std::vector<double>>(),
                                                                                                                             univs),
                fPDGToHighestDigitE(pdgCategories, dir, "HighestDigitE", "Highest Digit Energy;Number of Candidates",
                                    30, 0, 30, univs),
                fPDGToHighestEVersusNDigits(pdgCategories, dir, "HighestEVersusNDigits", "Number of Digits;Highest Energy Digit;Number of candidates",

                                          30, 0, 30,

                                          30, 0, 30,

                                          univs),
                                          fSingleDigitPi0Events("SingleDigitsPi0Events.txt")
  {
    constexpr int nBins = 30;
    fClosestEDepVersusDist = dir.make<LOGHIST2D>("ClosestEDepVersusDist", "Closest Candidate per FS;log(Distance from Vertex/1mm);log(Candidate Visible Energy/1MeV)", nBins, 3, 9, nBins, 0.2, 6, univs);

    fFartherEDepVersusDist = dir.make<LOGHIST2D>("FartherEDepVersusDist", "Not Closest Candidate per FS;log(Distance from Vertex/1mm);log(Candidate Visible Energy/1MeV)", nBins, 3, 9, nBins, 0.2, 6, univs);
  }

  void NeutronPurity::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    //Cache weight for each universe
    const neutrons weightPerNeutron = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_calo_edep(),
                                              event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(),
                                              event.Getblob_earliest_time(), event.Getblob_n_clusters(),
                                              event.Getblob_n_digits(), event.Getblob_highest_digit_E(),
                                              event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z());
    const auto vertex = event.GetVtx();

    std::unordered_map<int, std::vector<MCCandidate>> fsNeutronToCands; //Mapping from FS neutron to candidates it produced

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

          if(fCuts.countAsTruth(fs[cand.FS_index])) fsNeutronToCands[cand.FS_index].push_back(cand);
        }

        fPDGToEDepVersusNClusters[pdg].Fill(&event, Clusters(cand.nClusters), cand.caloEdep, weightPerNeutron);
        fPDGToEDepVersusNDigits[pdg].Fill(&event, Digits(cand.nDigits), cand.caloEdep, weightPerNeutron);
        fPDGToHighestDigitE[pdg].Fill(&event, cand.highestDigitE, weightPerNeutron);
        fPDGToHighestEVersusNDigits[pdg].Fill(&event, Digits(cand.nDigits), cand.highestDigitE, weightPerNeutron);

        if((pdg == 111) && (cand.nDigits == 1)) fSingleDigitPi0Events << util::arachne(event.GetEventID(false), false, true) << "\n";
      }
    }

    for(auto fs: fsNeutronToCands)
    {
      const auto closest = std::min_element(fs.second.begin(), fs.second.end(),
                                            [&vertex](const auto& lhs, const auto& rhs)
                                            {
                                              return distToVertex(lhs, vertex) < distToVertex(rhs, vertex);
                                            });

      if(closest != fs.second.end())
      {
        fClosestEDepVersusDist->FillUniverse(&event, log(distToVertex(*closest, vertex).in<mm>()), log(closest->caloEdep.in<MeV>()), weight.in<events>());
        fs.second.erase(closest); //N.B.: I'm modifying fsNeutronToCands here!
      }

      for(const auto& cand: fs.second) fFartherEDepVersusDist->FillUniverse(&event, log(distToVertex(cand, vertex).in<mm>()), log(cand.caloEdep.in<MeV>()), weight.in<events>());
    }
  }

  void NeutronPurity::afterAllFiles(const events passedSelection)
  {
    fPDGToEDepVersusNClusters.visit([passedSelection](auto& hist)
                                    {
                                      hist.SyncCVHistos();
                                    });
    fPDGToEDepVersusNDigits.visit([passedSelection](auto& hist)
                                  {
                                    hist.SyncCVHistos();
                                  });
    fPDGToHighestDigitE.visit([passedSelection](auto& hist)
                              {
                                hist.SyncCVHistos();
                              });
    fPDGToHighestEVersusNDigits.visit([passedSelection](auto& hist)
                                      {
                                        hist.SyncCVHistos();
                                      });
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::NeutronPurity> NeutronPurity_reg("NeutronPurity");
}
