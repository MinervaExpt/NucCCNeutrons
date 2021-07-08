//File: NSFValidation.h
//Brief: Plots 3 physics quantities for validating New Systematics Framework
//       systematic universe usage.  Run over CCQENu with an empty signal definition
//       and use the results with Rob's validation suite in PlotUtils.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//reweighters includes
#include "PlotUtils/FluxAndCVReweighter.h"
#include "PlotUtils/GENIEReweighter.h"
#include "PlotUtils/LowRecoil2p2hReweighter.h"
#include "PlotUtils/RPAReweighter.h"
#include "PlotUtils/MINOSEfficiencyReweighter.h"

//signal includes
#include "analyses/base/Study.h"

//util includes
#include "util/Categorized.h"
#include "util/WithUnits.h"

#ifndef ANA_NSFVALIDATION_H
#define ANA_NSFVALIDATION_H

namespace ana
{
  class NSFValidation: public Study
  {
    public:
      NSFValidation(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~NSFValidation() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      //Normalize fPDGToObservables and syncCVHistos()
      virtual void afterAllFiles(const events /*passedSelection*/) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {}; //TODO: Do I want to plot candidate observables in data?

      //I don't need the Truth loop
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      template <class UNIT>
      using HIST = units::WithUnits<HistWrapper<evt::Universe>, UNIT, events>;

      template <class UNIT>
      using HIST2D = units::WithUnits<Hist2DWrapper<evt::Universe>, UNIT, events, entries>;

      HIST<GeV>* fEMu; //Muon energy
      HIST<GeV>* fPTMu; //Muon transverse momentum
      HIST<MeV>* fRecoilE; //Recoil energy as defined by CCQENu

      //Histograms of physics quantities versus weights.
      //They help debug weights when they are different from the reference.
      HIST2D<GeV>* fPMINOSVersusMINOSWeight;
      HIST2D<GeV>* fEMuVersusMINOSWeight;
      HIST2D<GeV>* fEMuVersusGENIEWeight;
      HIST2D<GeV>* fEMuVersusRPAWeight;
      HIST2D<GeV>* fEMuVersus2p2hWeight;
      HIST2D<GeV>* fEMuVersusFluxWeight;

      //Reweighters I'm going to plot
      PlotUtils::FluxAndCVReweighter<evt::Universe> fFluxWeighter;
      PlotUtils::GENIEReweighter<evt::Universe> fGENIEWeighter;
      PlotUtils::RPAReweighter<evt::Universe> fRPAWeighter;
      PlotUtils::MINOSEfficiencyReweighter<evt::Universe> fMINOSWeighter;
      PlotUtils::LowRecoil2p2hReweighter<evt::Universe> f2p2hWeighter;
  };
}

#endif //ANA_NSFVALIDATION_H
