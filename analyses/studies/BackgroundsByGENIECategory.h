//File: BackgroundsByGENIECategory.h
//Brief: Plots Backgrounds for a VARIABLE with each background further broken down
//       by whether it has charged and/or neutral pions.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/Universe.h"

//signal includes
#include "analyses/base/Study.h"
#include "analyses/studies/TruthInteractionCategories.h"

//util includes
#include "util/Categorized.h"
#include "util/WithUnits.h"
#include "util/units.h"
#include "util/Directory.h"

//PlotUtils includes
//TODO: Someone who maintains this code should deal with these warnings
#pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                            //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#pragma GCC diagnostic pop

#ifndef ANA_BACKGROUNDSBYGENIECATEGORY_H
#define ANA_BACKGROUNDSBYGENIECATEGORY_H

namespace ana
{
  template <class VARIABLE>
  class BackgroundsByGENIECategory: public Study
  {
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::Universe>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::Universe>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::Universe>, UNIT, events>;

    public:
      BackgroundsByGENIECategory(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                               std::vector<background_t>& backgrounds,
                               std::map<std::string, std::vector<evt::Universe*>>& univs):
                               Study(config, dir, std::move(mustPass), backgrounds, univs),
                               fVar(config["variable"]),
                               fSignalByGENIEInVar("SignalByGENIECategories", "Reco " + fVar.name(), GENIECategories, dir,
                                                   config["binning"].as<std::vector<double>>(), univs),
                               fBackgroundsByGENIEInVar(backgrounds, dir, "Background", "Reco " + fVar.name(),
                                                        GENIECategories, dir, config["binning"].as<std::vector<double>>(), univs),
                               fSelectedByGENIEInVar("SelectedByGENIECategories", "Reco " + fVar.name(), GENIECategories, dir,
                                                   config["binning"].as<std::vector<double>>(), univs)
      {
      }

      virtual ~BackgroundsByGENIECategory() = default;

      virtual void mcSignal(const evt::Universe& event, const events weight) override
      {
        fSignalByGENIEInVar[event.GetInteractionType()].Fill(&event, fVar.reco(event), weight);
        fSelectedByGENIEInVar[event.GetInteractionType()].Fill(&event, fVar.reco(event), weight);
      }

      virtual void mcBackground(const evt::Universe& event, const background_t& background, const events weight) override
      {
        fBackgroundsByGENIEInVar[background][event.GetInteractionType()].Fill(&event, fVar.reco(event), weight);
        fSelectedByGENIEInVar[event.GetInteractionType()].Fill(&event, fVar.reco(event), weight);
      }

      //Normalize fPDGToObservables and syncCVHistos()
      virtual void afterAllFiles(const events /*passedSelection*/) override
      {
        fSignalByGENIEInVar.visit([](auto& hist) { hist.SyncCVHistos(); });
        fBackgroundsByGENIEInVar.visit([](auto& category) { category.visit([](auto& hist) { hist.SyncCVHistos(); }); });
      }

      //Functions I don't plan to use
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {}

      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {}

      using Registrar = Study::Registrar<BackgroundsByGENIECategory<VARIABLE>>;

    private:
      VARIABLE fVar;

      util::Categorized<HIST, GENIECategory> fSignalByGENIEInVar;
      util::Categorized<util::Categorized<HIST, GENIECategory>, background_t> fBackgroundsByGENIEInVar;
      util::Categorized<HIST, GENIECategory> fSelectedByGENIEInVar; //Because I'm lazy and don't really want to write another plotting script right now
  };
}

#endif //ANA_BACKGROUNDSBYGENIECATEGORY_H
