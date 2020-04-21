//File: NSFValidation.cpp
//Brief: Plots 3 physics quantities for validating New Systematics Framework
//       systematic universe usage.  Run over CCQENu with an empty signal definition
//       and use the results with Rob's validation suite in PlotUtils.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/NSFValidation.h"

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/Factory.cpp"

namespace ana
{
  NSFValidation::NSFValidation(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                               std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs)
  {
    //Bins chosen to match the NSF_ValidationSuite
    const std::vector<double> EMuBins{0, 1, 2, 3, 4, 5, 7, 9, 12, 15, 18, 22, 36, 50, 75, 100, 120};
    fEMu = dir.make<HIST<GeV>>("Emu", "Reco", EMuBins, univs);

    const std::vector<double> pTMuBins{0, 0.075, 0.15, 0.25, 0.325, 0.4, 0.475, 0.55, 0.7, 0.85, 1, 1.25, 1.5, 2.5, 4.5};
    fPTMu = dir.make<HIST<GeV>>("Pt", "Reco", pTMuBins, univs);

    fRecoilE = dir.make<HIST<MeV>>("Nu", "Reco", 100, 0, 5000, univs);

    fPMINOSVersusMINOSWeight = dir.make<HIST2D<GeV>>("WgtVsPmuMinos_MINOS", "Reco;Weight", 40, 0, 20, 100, 0.9, 1.1, univs);

    fEMuVersusMINOSWeight = dir.make<HIST2D<GeV>>("WgtVsEmu_MINOS", "Reco;Weight", 40, 0, 20, 100, 0.9, 1.1, univs);

    fEMuVersusGENIEWeight = dir.make<HIST2D<GeV>>("WgtVsEmu_GENIE", "Reco;Weight", 40, 0, 20, 200, 0, 2, univs);

    fEMuVersusRPAWeight = dir.make<HIST2D<GeV>>("WgtVsEmu_RPA", "Reco;Weight", 40, 0, 20, 200, 0, 2, univs);

    fEMuVersus2p2hWeight = dir.make<HIST2D<GeV>>("WgtVsEmu_2p2h", "Reco;Weight", 40, 0, 20, 200, 0, 2, univs);

    fEMuVersusFluxWeight = dir.make<HIST2D<GeV>>("WgtVsEmu_Flux", "Reco;Weight", 40, 0, 20, 200, 0, 2, univs);
  }

  void NSFValidation::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    const auto muonP = event.GetMuonP();
    const GeV muonE = muonP.E();
    const units::XYZVector<double> zHat{0, 0, 1};

    fEMu->Fill(&event, muonE, weight);
    fPTMu->Fill(&event, /*muonP.p().cross(zHat).mag()*/muonP.p().mag()*sin(event.GetMuonTheta()), weight);
    fRecoilE->Fill(&event, event.GetRecoilE(), weight); //N.B.: My GetRecoilE() might not always match the NSF_ValidationSuite

    fPMINOSVersusMINOSWeight->Fill(&event, MeV(event.GetPmuMinos()), events(event.GetMinosEfficiencyWeight()), 1_entries);
    fEMuVersusMINOSWeight->Fill(&event, muonE, events(event.GetMinosEfficiencyWeight()), 1_entries);
    fEMuVersusGENIEWeight->Fill(&event, muonE, events(event.GetGenieWeight()), 1_entries);
    fEMuVersusRPAWeight->Fill(&event, muonE, events(event.GetRPAWeight()), 1_entries);
    fEMuVersus2p2hWeight->Fill(&event, muonE, events(event.GetLowRecoil2p2hWeight()), 1_entries);
    fEMuVersusFluxWeight->Fill(&event, muonE, events(event.GetFluxAndCVWeight()), 1_entries);
  }

  void NSFValidation::afterAllFiles(const events /*passedSelection*/)
  {
    fEMu->SyncCVHistos();
    fPTMu->SyncCVHistos();
    fRecoilE->SyncCVHistos();

    fPMINOSVersusMINOSWeight->SyncCVHistos();
    fEMuVersusMINOSWeight->SyncCVHistos();
    fEMuVersusGENIEWeight->SyncCVHistos();
    fEMuVersusRPAWeight->SyncCVHistos();
    fEMuVersus2p2hWeight->SyncCVHistos();
    fEMuVersusFluxWeight->SyncCVHistos();
  }
}

namespace
{
  static ana::Study::Registrar<ana::NSFValidation> NSFValidation_reg("NSFValidation");
}
