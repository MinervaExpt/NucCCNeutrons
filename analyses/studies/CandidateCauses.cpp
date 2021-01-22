//File: CandidateCauses.h
//Brief: A Study to help understand the secondary particles that deposit the energy in neutron candidates.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/CandidateCauses.h"

//util includes
#include "util/Factory.cpp"

//ROOT includes
#include "TDatabasePDG.h"

namespace
{
  std::vector<util::NamedCategory<int>> pdgToName = {util::NamedCategory<int>{{2212}, "Proton"},
                                                     util::NamedCategory<int>{{2112}, "Neutron"},
                                                     util::NamedCategory<int>{{22}, "Gamma"},
                                                     //util::NamedCategory<int>{{211, -211}, "Charged Pion"},
                                                     //util::NamedCategory<int>{{11, -11}, "Electron"},
                                                     util::NamedCategory<int>{{1000010020}, "Deuteron"},
                                                     util::NamedCategory<int>{{1000020040}, "Alpha"},
                                                     util::NamedCategory<int>{{1000060120}, "Carbon"},
                                                     util::NamedCategory<int>{{1000050100}, "Boron"},
                                                     util::NamedCategory<int>{{1000010030}, "Tritium"},
                                                     util::NamedCategory<int>{{1000040080, 1000040090, 1000040100}, "Beryllium"},
                                                    };
}

namespace ana
{
  CandidateCauses::CandidateCauses(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                   std::vector<background_t>& backgrounds, std::map<std::string,
                                   std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                         fCuts(config["variable"]),
                                                                         fPDGCodeToCauseEnergy(::pdgToName, dir, "Cause Energy",
                                                                                               "Truth", 30, 0, 50, univs)
  {
    fCauseNames = dir.make<PlotUtils::HistWrapper<evt::Universe>>("CauseNames", "Particle that Caused a Candidate", 1, 0, -1, univs);
    fCauseNameVersusEDep = dir.make<PlotUtils::Hist2DWrapper<evt::Universe>>("CauseNamesVsEDep", "Cause Versus Energy Deposit;Reco EDep [MeV];Cause Name;events", 39, 0, 65, 1, 0, -1, univs);

    //TDatabasePDG isn't smart enough to handle nuclei easily,
    //and I forgot the name of the class that handles elements.
    for(const auto& entry: ::pdgToName)
    {
      for(const auto pdg: entry.values) fPDGToName[pdg] = entry.name;
    }
  }

  void CandidateCauses::mcSignal(const evt::Universe& event, const events weight)
  {
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex(),
                                              event.Getblob_n_causes(), event.Getblob_FS_index(),
                                              event.Getblob_geant_dist_to_edep_as_neutron());
    const auto causes = event.Get<Cause>(event.GetBlobCausePDGs(), event.GetBlobCauseEnergies());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy());

    int startCause = 0;
    for(const auto& cand: cands)
    {
      const int endCause = startCause + cand.nCauses;

      //Only interested in candidates from neutrons, be they FS or "GEANT neutrons"
      if((cand.FS_index > 0 && fCuts.countAsTruth(fs[cand.FS_index])) || cand.dist_to_edep_as_neutron > 0_mm)
      {
        for(int whichCause = startCause; whichCause < endCause; ++whichCause)
        {
          const auto& cause = causes[whichCause];

          const auto causeName = fPDGToName[cause.pdgCode];
          fCauseNames->univHist(&event)->Fill(causeName.c_str(), weight.in<events>());
          fCauseNameVersusEDep->univHist(&event)->Fill(cand.edep.in<MeV>(), causeName.c_str(), weight.in<events>());
          fPDGCodeToCauseEnergy[cause.pdgCode].Fill(&event, cause.energy, weight);
        }
      }

      startCause = endCause;
    }
  }

  void CandidateCauses::afterAllFiles(const events /*passedSelection*/)
  {
    fPDGCodeToCauseEnergy.visit([](auto& hist) { hist.SyncCVHistos(); });
    fCauseNames->SyncCVHistos();
    fCauseNameVersusEDep->SyncCVHistos();
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::CandidateCauses> CandidateCauses_reg("CandidateCauses");
}
