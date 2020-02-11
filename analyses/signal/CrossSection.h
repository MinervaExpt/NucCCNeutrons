//File: CrossSection.h
//Brief: A Signal template that just produces the plots you need
//       to extract a differential cross section in 1D (TODO in an arbitrary
//       number of dimensions).  Use it to implement a CrossSection
//       for your VARIABLE of interest.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/signal/Signal.h"

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
#include "HistWrapper.h"
#include "Hist2DWrapper.h"
#pragma GCC diagnostic pop

namespace sig
{
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class CrossSection: public Signal
  {
    //First, check that VARIABLE makes sense
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(std::declval<VARIABLE>().truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = units::WithUnits<HistWrapper<evt::CVUniverse>, UNIT, events>;
      using MIGRATION = units::WithUnits<Hist2DWrapper<evt::CVUniverse>, UNIT, UNIT, events>;

    public:
      CrossSection(const YAML::Node& config, util::Directory& dir, std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::CVUniverse*>>& universes): Signal(config, dir, backgrounds, universes),
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

      virtual ~CrossSection() = default;

      virtual void mcSignal(const std::vector<evt::CVUniverse*>& univs) override
      {
        const auto reco = fVar.reco(*univs.front()), truth = fVar.truth(*univs.front());

        for(const auto univ: univs)
        {
          fEfficiencyNum->Fill(univ, truth, univ->GetWeight());
          fMigration->Fill(univ, reco, truth, univ->GetWeight());
        }
      }

      virtual void truth(const std::vector<evt::CVUniverse*>& univs) override
      {
        const auto truth = fVar.truth(*univs.front());

        for(const auto univ: univs) fEfficiencyDenom->Fill(univ, truth, univ->GetWeight());
      }

      virtual void data(const std::vector<evt::CVUniverse*>& univs) override
      {
        const auto reco = fVar.reco(*univs.front());

        for(const auto univ: univs) fSignalEvents->Fill(univ, reco);
      }

      virtual void mcBackground(const std::vector<evt::CVUniverse*>& univs, const background_t& background) override
      {
        const auto reco = fVar.reco(*univs.front());

        for(const auto univ: univs) fBackgrounds[background].Fill(univ, reco, univ->GetWeight());
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
