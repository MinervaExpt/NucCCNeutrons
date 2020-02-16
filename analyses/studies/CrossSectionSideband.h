//File: CrossSectionSideband.h
//Brief: The sideband plots needed to extract a cross section in a single
//       VARIABLE (TODO: in multiple dimensions).  A sideband is defined by
//       1 or more reconstruction cuts that an event fails and can be delimited
//       by that event passes a set of selection cuts.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/units.h"
#include "util/WithUnits.h"
#include "util/Directory.h"
#include "util/Categorized.h"

#ifndef ANA_CROSSSECTIONSIDEBAND_H
#define ANA_CROSSSECTIONSIDEBAND_H

namespace ana
{
  //This is the same concept of VARIABLE as used in analyses/signal/CrossSectionSideband.h and
  //all other CrossSeciton headers.
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class CrossSectionSideband: public Study
  {
    //Sanity checks on VARIABLE
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::CVUniverse>, UNIT, events>;

    public:
      CrossSectionSideband(const YAML::Node& config, util::Directory& dir,
                   cuts_t&& passes, const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::CVUniverse*>>& universes): Study(config, dir, std::move(passes), backgrounds, universes),
                                                                                     fVar(config["variable"]),
                                                                                     fBackgrounds(backgrounds, dir, "Background", "Reco " + fVar.name(),
                                                                                                  config["binning"].as<std::vector<double>>(), universes)
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?

        fData = dir.make<HIST>("Data", ("Data;Reco " + fVar.name() + ";entries").c_str(),
                               binning, universes);
        fSignal = dir.make<HIST>("TruthSignal", ("Truth Signal;Reco " + fVar.name() + ";entries").c_str(),
                                 binning, universes);
      }

      virtual ~CrossSectionSideband() = default;

      virtual void data(const evt::CVUniverse& event) override
      {
        fData->Fill(&event, fVar.reco(event)); //No weight applied to data
      }

      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override
      {
        fSignal->Fill(&event, fVar.reco(event), weight);
      }

      virtual void mcBackground(const evt::CVUniverse& event, const background_t& background, const events weight) override
      {
        fBackgrounds[background].Fill(&event, fVar.reco(event), weight);
      }

      //Sidebands aren't defined in the truth tree, so do nothing
      virtual void truth(const evt::CVUniverse& /*event*/, const events /*weight*/) override
      {
      }

      virtual void afterAllFiles() override
      {
        fData->SyncCVHistos();
        fSignal->SyncCVHistos();
        fBackgrounds.visit([](auto& hist) { hist.SyncCVHistos();});
      }

      using Registrar = Study::Registrar<CrossSectionSideband<VARIABLE>>;

    private:
      VARIABLE fVar;

      //Histograms for cross section extraction
      //These plots are all used together, and one of them is filled with data.
      //So, they're all in reco variables.
      HIST* fData; //TODO: This can be a TH1D
      HIST* fSignal;
      util::Categorized<HIST, background_t> fBackgrounds;
  };
}

#endif //ANA_CROSSSECTIONSIDEBAND_H
