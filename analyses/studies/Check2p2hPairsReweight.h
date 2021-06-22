//File: Check2p2hPairsReweight.h
//Brief: Plots 3 physics quantities for validating New Systematics Framework
//       systematic universe usage.  Run over CCQENu with an empty signal definition
//       and use the results with Rob's validation suite in PlotUtils.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/EAvailable.cpp"

//signal includes
#include "analyses/base/Study.h"

//util includes
#include "util/Categorized.h"
#include "util/WithUnits.h"

#ifndef ANA_NSFVALIDATION_H
#define ANA_NSFVALIDATION_H

namespace ana
{
  class Check2p2hPairsReweight: public Study
  {
    public:
      Check2p2hPairsReweight(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~Check2p2hPairsReweight() = default;

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
      template <class XUNIT, class YUNIT>
      using HIST2D = units::WithUnits<PlotUtils::Hist2DWrapper<evt::Universe>, XUNIT, YUNIT, events>;

      PlotUtils::HistWrapper<evt::Universe>* fTargetNucleons;
      util::Categorized<HIST2D<MeV, MeV>, int> fEAvailSmearingByNucleonPair;
      HIST2D<MeV, MeV>* fEAvailSmearingAll2p2h;
      util::Categorized<HIST2D<GeV, GeV>, int> fQ0Q3ByNucleonPair;
      HIST2D<GeV, GeV>* fQ0Q3Overall;

      ana::EAvailable fVariable;
  };
}

#endif //ANA_NSFVALIDATION_H
