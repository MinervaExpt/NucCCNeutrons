//File: CrossSectionSignal.h
//Brief: A Study template that just produces the plots you need
//       to extract a differential cross section in 1D (TODO in an arbitrary
//       number of dimensions).  Use it to implement a CrossSectionSignal
//       for your VARIABLE of interest.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/Universe.h"

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

#ifndef ANA_CROSSSECTIONSIGNAL_H
#define ANA_CROSSSECTIONSIGNAL_H

namespace ana
{
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const Universe& univ) const method
  //3) A UNIT truth(const Universe& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class CrossSectionSignal: public Study
  {
    //First, check that VARIABLE makes sense
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::Universe>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::Universe>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::Universe>, UNIT, events>;
      using MIGRATION = units::WithUnits<Hist2DWrapper<evt::Universe>, UNIT, UNIT, events>;

    public:
      CrossSectionSignal(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::Universe*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes),
                                                                                     fVar(config["variable"]),
                                                                                     fBackgrounds(backgrounds, dir, "Background", "Reco " + fVar.name(),
                                                                                                  config["binning"].as<std::vector<double>>(), universes)
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?
        std::vector<double> truthBinning = binning;
        if(config["truthBinning"]) truthBinning = config["truthBinning"].as<std::vector<double>>(); //Expert interface for Hang's warping studies with non-square migration matrices

        fMigration = dir.make<MIGRATION>("Migration", ("Migration;Reco " + fVar.name() + ";Truth " + fVar.name() + ";entries").c_str(),
                                         binning, truthBinning, universes);
        fSignalEvents = dir.make<HIST>("Signal", ("Signal;Reco " + fVar.name() + ";entries").c_str(),
                                       binning, universes);
        fEfficiencyNum = dir.make<HIST>("EfficiencyNumerator", ("Efficiency Numerator;Truth " + fVar.name() + ";entries").c_str(),
                                        truthBinning, universes);
        fEfficiencyDenom = dir.make<HIST>("EfficiencyDenominator", ("Efficiency Denominator;Truth " + fVar.name() + ";entries").c_str(),
                                          truthBinning, universes);
        fSelectedMCEvents = dir.make<HIST>("SelectedMCEvents", ("Selected Signal Events;Reco " + fVar.name() + ";entries").c_str(),
                                           binning, universes);

        //Write out the flux integral.  I want to have each bin in my variable filled
        //in with the total flux integral so I can just MnvH1D::Divide() by it.
        auto cv = universes["cv"].front();
        if(cv) //Only case I can think of where this fails is debugging a specific error band.
        {
          auto fluxIntegral = universes["cv"].front()->GetFluxIntegral(*fEfficiencyNum); //*fSelectedMCEvents);
          dir.mv(fluxIntegral);
        }
      }

      virtual ~CrossSectionSignal() = default;

      virtual void mcSignal(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt) override
      {
        assert(!univs.empty());
        const auto reco = fVar.reco(*univs.front()), truth = fVar.truth(*univs.front());

        fEfficiencyNum->Fill(univs, truth, model, evt);
        fMigration->Fill(univs, reco, truth, model, evt);
        fSelectedMCEvents->Fill(univs, reco, model, evt);
      }

      virtual void truth(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt) override
      {
        assert(!univs.empty());
        fEfficiencyDenom->Fill(univs, fVar.truth(*univs.front()), model, evt);
      }

      virtual void data(const evt::Universe& event, const events weight) override
      {
        fSignalEvents->Fill(&event, fVar.reco(event), weight);
      }

      virtual void mcBackground(const std::vector<evt::Universe*>& univs, const background_t& background, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt) override
      {
        assert(!univs.empty());
        fBackgrounds[background].Fill(univs, fVar.reco(*univs.front()), model, evt);
      }

      virtual void afterAllFiles(const events /*passedSelection*/) override
      {
        fMigration->SyncCVHistos();
        fEfficiencyNum->SyncCVHistos();
        fEfficiencyDenom->SyncCVHistos();
        fBackgrounds.visit([](auto& hist) { hist.SyncCVHistos(); });
        fSelectedMCEvents->SyncCVHistos();
      }

      using Registrar = Study::Registrar<CrossSectionSignal<VARIABLE>>;

    private:
      VARIABLE fVar;  //VARIABLE in which a differential cross section will be extracted

      //Signal histograms needed to extract a cross section
      MIGRATION* fMigration;
      HIST* fSignalEvents; //TODO: make this just a TH1D.  Change the interface to data() appropriately.
      HIST* fEfficiencyNum;
      HIST* fEfficiencyDenom;

      util::Categorized<HIST, background_t> fBackgrounds; //Background event distributions in the reco signal region

      //Not needed for a cross section.  Add() to fBackgrounds to get total reco event selection breakdown.
      HIST* fSelectedMCEvents;
  };
}

#endif //ANA_CROSSSECTIONSIGNAL_H
