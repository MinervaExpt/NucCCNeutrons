//File: CrossSection2DSignal.h
//Brief: A Study template that just produces the plots you need
//       to extract a differential cross section in 1D (TODO in an arbitrary
//       number of dimensions).  Use it to implement a CrossSection2DSignal
//       for your VARIABLE of interest.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/WithUnits.h"
#include "util/units.h"
#include "util/Directory.h"
#include "util/Categorized.h"

//PlotUtils includes
//TODO: Someone who maintains this code should deal with these warnings
#pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                            //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#pragma GCC diagnostic pop

#ifndef ANA_CROSSSECTION2DSIGNAL_H
#define ANA_CROSSSECTION2DSIGNAL_H

namespace ana
{
  //TODO: Could I generalize CrossSection<> for a variable number of arguments with HyperDimLinearizer?
  //      I'd imagine that the 1D and 2D versions of HIST would be special cases via some template specialization.
  //      If I knew how to use HyperDimLinearizer, the one remaining challenge I see is holding one of each VARIABLE type.
  //      std::tuple<> could do that job at a cost.  VARIABLEs should be unique, so I could even do this with mutiple inheritance.
  //      Tabling this because I have no plans to use it.  It would be very interesting though!

  //A VARIABLE (XVAR or YVAR) shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match
  template <class XVAR, class YVAR>
  class CrossSection2DSignal: public Study
  {
    //First, check that VARIABLE makes sense
    private:
      using XUNIT = decltype(std::declval<XVAR>().reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<XUNIT, decltype(std::declval<XVAR>().truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth x variable calculations must be in the same units!");

      using YUNIT = decltype(std::declval<YVAR>().reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<YUNIT, decltype(std::declval<YVAR>().truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth y variable calculations must be in the same units!");

      using HIST = units::WithUnits<Hist2DWrapper<evt::CVUniverse>, XUNIT, YUNIT, events>;
      //using MIGRATION = units::WithUnits<Hist2DWrapper<evt::CVUniverse>, XUNIT, XUNIT, YUNIT, YUNIT, events>;

    public:
      CrossSection2DSignal(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::CVUniverse*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes),
                                                                                     fXVar(config["variable"]["x"]),
                                                                                     fYVar(config["variable"]["y"]),
                                                                                     fBackgrounds(backgrounds, dir, "Background", "Reco " + fXVar.name()
                                                                                                  + ";Reco " + fYVar.name(),
                                                                                                  config["binning"]["x"].as<std::vector<double>>(),
                                                                                                  config["binning"]["y"].as<std::vector<double>>(),
                                                                                                  universes)
      {
        const auto xBins = config["binning"]["x"].as<std::vector<double>>(),
                   yBins = config["binning"]["y"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?

        /*fMigration = dir.make<MIGRATION>("Migration", ("Migration;Reco " + fVar.name() + ";Truth " + fVar.name() + ";entries").c_str(),
                                         binning, binning, universes);*/
        fSignalEvents = dir.make<HIST>("Signal", ("Signal;Reco " + fXVar.name() + ";" + fYVar.name() + ";entries").c_str(),
                                       xBins, yBins, universes);
        fEfficiencyNum = dir.make<HIST>("EfficiencyNumerator", ("Efficiency Numerator;Truth " + fXVar.name() + ";Truth " + fYVar.name() + ";entries").c_str(),
                                        xBins, yBins, universes);
        fEfficiencyDenom = dir.make<HIST>("EfficiencyDenominator", ("Efficiency Denominator;Truth " + fXVar.name() + ";Truth " + fYVar.name() + ";entries").c_str(),
                                          xBins, yBins, universes);
        fSelectedMCEvents = dir.make<HIST>("SelectedMCEvents", ("Selected Signal Events;Reco " + fXVar.name() + ";Reco " + fYVar.name() + ";entries").c_str(),
                                           xBins, yBins, universes);
      }

      virtual ~CrossSection2DSignal() = default;

      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override
      {
        const auto recoX = fXVar.reco(event), truthX = fXVar.truth(event);
        const auto recoY = fYVar.reco(event), truthY = fYVar.truth(event);

        fEfficiencyNum->Fill(&event, truthX, truthY, weight);
        //fMigration->Fill(&event, recoX, truthX, recoY, truthY, weight);
        fSelectedMCEvents->Fill(&event, recoX, recoY, weight);
      }

      virtual void truth(const evt::CVUniverse& event, const events weight) override
      {
        fEfficiencyDenom->Fill(&event, fXVar.truth(event), fYVar.truth(event), weight);
      }

      virtual void data(const evt::CVUniverse& event) override
      {
        fSignalEvents->Fill(&event, fXVar.reco(event), fYVar.reco(event)); //No weight applied to data
      }

      virtual void mcBackground(const evt::CVUniverse& event, const background_t& background, const events weight) override
      {
        fBackgrounds[background].Fill(&event, fXVar.reco(event), fYVar.reco(event), weight);
      }

      virtual void afterAllFiles(const events /*passedSelection*/) override
      {
        //fMigration->SyncCVHistos();
        fEfficiencyNum->SyncCVHistos();
        fEfficiencyDenom->SyncCVHistos();
        fBackgrounds.visit([](auto& hist) { hist.SyncCVHistos(); });
        fSelectedMCEvents->SyncCVHistos();
      }

      using Registrar = Study::Registrar<CrossSection2DSignal<XVAR, YVAR>>;

    private:
      //VARIABLE in which a differential cross section will be extracted
      XVAR fXVar;
      YVAR fYVar;

      //Signal histograms needed to extract a cross section
      //MIGRATION* fMigration; //TODO: How to fill and write a 4D migration matrix?  Do I need HyperDimLinearizer?  Dan says I can use MnvResponse
      HIST* fSignalEvents; //TODO: make this just a TH2D.  Change the interface to data() appropriately.
      HIST* fEfficiencyNum;
      HIST* fEfficiencyDenom;

      util::Categorized<HIST, background_t> fBackgrounds; //Background event distributions in the reco signal region

      //Not needed for a cross section.  Add() to fBackgrounds to get total reco event selection breakdown.
      HIST* fSelectedMCEvents;
  };
}

#endif //ANA_CROSSSECTION2DSIGNAL_H
