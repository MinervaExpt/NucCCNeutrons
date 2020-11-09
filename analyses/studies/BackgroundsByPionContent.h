//File: BackgroundsByPionContent.h
//Brief: Plots Backgrounds for a VARIABLE with each background further broken down
//       by whether it has charged and/or neutral pions.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/CVUniverse.h"

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

#ifndef ANA_BACKGROUNDSBYPIONCONTENT_H
#define ANA_BACKGROUNDSBYPIONCONTENT_H

namespace ana
{
  template <class VARIABLE>
  class BackgroundsByPionContent: public Study
  {
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::CVUniverse>, UNIT, events>;

    public:
      BackgroundsByPionContent(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                               std::vector<background_t>& backgrounds,
                               std::map<std::string, std::vector<evt::CVUniverse*>>& univs):
                               Study(config, dir, std::move(mustPass), backgrounds, univs),
                               fVar(config["variable"]),
                               fSignalByPionsInVar(pionFSCategories, dir, "SignalByPionCategories", "Reco " + fVar.name(),
                                                   config["binning"].as<std::vector<double>>(), univs),
                               fBackgroundsByPionsInVar(backgrounds, dir, "Background", "Reco " + fVar.name(),
                                                        pionFSCategories, dir, config["binning"].as<std::vector<double>>(), univs)
      {
      }

      virtual ~BackgroundsByPionContent() = default;

      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override
      {
        const auto pionCat = std::find_if(pionFSCategories.begin(), pionFSCategories.end(), [&event](auto& category) { return (*category)(event); });
        fSignalByPionsInVar[*pionCat].Fill(&event, fVar.reco(event), weight);
      }

      virtual void mcBackground(const evt::CVUniverse& event, const background_t& background, const events weight) override
      {
        const auto pionCat = std::find_if(pionFSCategories.begin(), pionFSCategories.end(), [&event](auto& category) { return (*category)(event); });
        fBackgroundsByPionsInVar[background][*pionCat].Fill(&event, fVar.reco(event), weight);
      }

      //Normalize fPDGToObservables and syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override
      {
        fSignalByPionsInVar.visit([](auto& hist) { hist.SyncCVHistos(); });
        fBackgroundsByPionsInVar.visit([](auto& category) { category.visit([](auto& hist) { hist.SyncCVHistos(); }); });
      }

      //Functions I don't plan to use
      virtual void truth(const evt::CVUniverse& /*event*/, const events /*weight*/) override {}

      virtual void data(const evt::CVUniverse& /*event*/) override {}

      using Registrar = Study::Registrar<BackgroundsByPionContent<VARIABLE>>;

    private:
      VARIABLE fVar;

      util::Categorized<HIST, FSCategory*> fSignalByPionsInVar;
      util::Categorized<util::Categorized<HIST, FSCategory*>, background_t> fBackgroundsByPionsInVar;
  };
}

#endif //ANA_BACKGROUNDSBYPIONCONTENT_H
