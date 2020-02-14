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

#ifndef ANA_CROSSSECTIONSIGNAL_H
#define ANA_CROSSSECTIONSIGNAL_H

namespace ana
{
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class CrossSectionSignal: public Study
  {
    //First, check that VARIABLE makes sense
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::CVUniverse>, UNIT, events>;
      using MIGRATION = units::WithUnits<Hist2DWrapper<evt::CVUniverse>, UNIT, UNIT, events>;

    public:
      CrossSectionSignal(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::CVUniverse*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes),
                                                                                     fVar(config["variable"]),
                                                                                     fBackgrounds(backgrounds, dir, "Background", "Reco " + fVar.name(),
                                                                                                  config["binning"].as<std::vector<double>>(), universes)
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?

        fMigration = dir.make<MIGRATION>("Migration", ("Migration;Reco " + fVar.name() + ";Truth " + fVar.name() + ";entries").c_str(),
                                         binning, binning, universes);
        fSignalEvents = dir.make<HIST>("Signal", ("Signal;Reco " + fVar.name() + ";entries").c_str(),
                                       binning, universes);
        fEfficiencyNum = dir.make<HIST>("EfficiencyNumerator", ("Efficiency Numerator;Truth " + fVar.name() + ";entries").c_str(),
                                        binning, universes);
        fEfficiencyDenom = dir.make<HIST>("EfficiencyDenominator", ("Efficiency Denominator;Truth " + fVar.name() + ";entries").c_str(),
                                          binning, universes);
      }

      virtual ~CrossSectionSignal() = default;

      virtual void mcSignal(const evt::CVUniverse& event) override
      {
        const auto reco = fVar.reco(event), truth = fVar.truth(event);
        const auto weight = event.GetWeight();

        fEfficiencyNum->Fill(&event, truth, weight);
        fMigration->Fill(&event, reco, truth, weight);
      }

      virtual void truth(const evt::CVUniverse& event) override
      {
        fEfficiencyDenom->Fill(&event, fVar.truth(event), event.GetWeight());
      }

      virtual void data(const evt::CVUniverse& event) override
      {
        fSignalEvents->Fill(&event, fVar.reco(event)); //No weight applied to data
      }

      virtual void mcBackground(const evt::CVUniverse& event, const background_t& background) override
      {
        fBackgrounds[background].Fill(&event, fVar.reco(event), event.GetWeight());
      }

    private:
      VARIABLE fVar;  //VARIABLE in which a differential cross section will be extracted

      //Signal histograms needed to extract a cross section
      MIGRATION* fMigration;
      HIST* fSignalEvents; //TODO: make this just a TH1D.  Change the interface to data() appropriately.
      HIST* fEfficiencyNum;
      HIST* fEfficiencyDenom;

      util::Categorized<HIST, background_t> fBackgrounds; //Background event distributions in the reco signal region
  };
}

#endif //ANA_CROSSSECTIONSIGNAL_H
