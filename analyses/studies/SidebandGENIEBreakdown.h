//File: SidebandGENIEBreakdown.h
//Brief: Shows what GENIE labels end up in each background of a sideband.
//       Useful for showing that a particular sideband is constraining just
//       1 of several unrelated cross section models from our generator.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//study includes
#include "analyses/studies/TruthInteractionCategories.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/Universe.h"

//util includes
#include "util/units.h"
#include "util/WithUnits.h"
#include "util/Directory.h"
#include "util/Categorized.h"

#ifndef ANA_SIDEBANDGENIEBREAKDOWN_H
#define ANA_SIDEBANDGENIEBREAKDOWN_H

namespace ana
{
  //This is the same concept of VARIABLE as used in analyses/signal/SidebandGENIEBreakdown.h and
  //all other CrossSeciton headers.
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const Universe& univ) const method
  //3) A UNIT truth(const Universe& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class SidebandGENIEBreakdown: public Study
  {
    //Sanity checks on VARIABLE
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::Universe>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::Universe>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::Universe>, UNIT, events>;
      using HIST2D = units::WithUnits<Hist2DWrapper<evt::Universe>, GeV, GeV, events>;

    public:
      SidebandGENIEBreakdown(const YAML::Node& config, util::Directory& dir,
                   cuts_t&& passes, const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::Universe*>>& universes): Study(config, dir, std::move(passes), backgrounds, universes),
                                                                                   fVar(config["variable"]),
                                                                                   fSignalByGENIELabel("TruthSignal", "Reco " + fVar.name(), GENIECategories, dir,
                                                                                                       config["binning"].as<std::vector<double>>(), universes),
                                                                                   fBackgroundsByGENIELabel(backgrounds, dir, "Background", "Reco " + fVar.name(),
                                                                                                            GENIECategories, dir, config["binning"].as<std::vector<double>>(), universes),
                                                                                   fBackgroundQ0Q3("Background_q0q3", "Truth q0;Truth q3", backgrounds, dir, 20, 0, 3, 20, 0, 3, universes)
      {
        fSignalQ0Q3 = dir.make<HIST2D>("Signal_q0q3", "Signal Phase Space;Truth q0;Truth q3", 20, 0, 3, 20, 0, 3, universes);
      }

      virtual ~SidebandGENIEBreakdown() = default;

      //GENIE labels are not available for data, so don't do anything.
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override
      {
      }

      virtual void mcSignal(const evt::Universe& event, const events weight) override
      {
        fSignalByGENIELabel[event.GetInteractionType()].Fill(&event, fVar.reco(event), weight);
        fSignalQ0Q3->Fill(&event, event.GetTruthQ0(), event.GetTruthQ3(), weight);
      }

      virtual void mcBackground(const evt::Universe& event, const background_t& background, const events weight) override
      {
        fBackgroundsByGENIELabel[background][event.GetInteractionType()].Fill(&event, fVar.reco(event), weight);
        fBackgroundQ0Q3[background].Fill(&event, event.GetTruthQ0(), event.GetTruthQ3());
      }

      //Sidebands aren't defined in the truth tree, so do nothing
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override
      {
      }

      virtual void afterAllFiles(const events /*passedSelection*/) override
      {
        fSignalByGENIELabel.visit([](auto& hist) { hist.SyncCVHistos(); });
        fSignalQ0Q3->SyncCVHistos();
        fBackgroundsByGENIELabel.visit([](auto& category) { category.visit([](auto& hist) { hist.SyncCVHistos(); }); });
        fBackgroundQ0Q3.visit([](auto& hist) { hist.SyncCVHistos(); });
      }

      using Registrar = Study::Registrar<SidebandGENIEBreakdown<VARIABLE>>;

    private:
      VARIABLE fVar;

      //Sideband histograms are designed to be subtracted from the selected region before
      //unfolding, so they must all be in reconstructed variables.
      util::Categorized<HIST, int> fSignalByGENIELabel; //Are the same GENIE labels dominant in the signal region and the sidebands?
      HIST2D* fSignalQ0Q3; //Do the signal and sidebands cover the same q0, q3 phase space? 

      util::Categorized<util::Categorized<HIST, int>, background_t> fBackgroundsByGENIELabel; //Are the same GENIE labels dominant in the signal region and the sidebands?
      util::Categorized<HIST2D, background_t> fBackgroundQ0Q3; //Do the signal and sidebands cover the same q0, q3 phase space?
  };
}

#endif //ANA_SIDEBANDGENIEBREAKDOWN_H
