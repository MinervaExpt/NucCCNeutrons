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
    fEMu = dir.make<HIST<GeV>>("h_inclusive_Emu", "Reco", EMuBins, univs);

    const std::vector<double> pTMuBins{0, 0.075, 0.15, 0.25, 0.325, 0.4, 0.475, 0.55, 0.7, 0.85, 1, 1.25, 1.5, 2.5, 4.5};
    fPTMu = dir.make<HIST<GeV>>("h_inclusive_Pt", "Reco", pTMuBins, univs);

    fRecoilE = dir.make<HIST<MeV>>("h_inclusive_Nu", "Reco", 50, 0, 5050, univs);
  }

  void NSFValidation::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    const auto muonP = event.GetMuonP();
    const units::XYZVector<double> zHat{0, 0, 1};

    fEMu->Fill(&event, muonP.E(), weight);
    fPTMu->Fill(&event, muonP.p().cross(zHat).mag(), weight);
    fRecoilE->Fill(&event, event.GetRecoilE(), weight); //N.B.: My GetRecoilE() might not always match the NSF_ValidationSuite
  }

  void NSFValidation::afterAllFiles(const events /*passedSelection*/)
  {
    fEMu->SyncCVHistos();
    fPTMu->SyncCVHistos();
    fRecoilE->SyncCVHistos();
  }
}

namespace
{
  static ana::Study::Registrar<ana::NSFValidation> NSFValidation_reg("NSFValidation");
}
