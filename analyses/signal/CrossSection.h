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
      CrossSection(const YAML::Node& config, util::Directory& dir, std::vector<evt::CVUniverse*>& universes): Signal(config, dir, universes),
                                                                                                              fVar(config["variable"])
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?

        fMigration.reset(dir.make<MIGRATION>("Migration", ("Migration;Reco " + fVar.name() + ";Truth " + fVar.name() + ";entries").c_str(),
                                             binning, binning, universes));
        fSignalEvents.reset(dir.make<HIST>("Signal", ("Signal;" + fVar.name() + ";entries").c_str(),
                                           binning, universes));
        fEfficiencyNum.reset(dir.make<HIST>("EfficiencyNumerator", ("Efficiency Numerator;" + fVar.name() + ";entries").c_str(),
                                            binning, universes));
        fEfficiencyDenom.reset(dir.make<HIST>("EfficiencyDenominator", ("Efficiency Denominator;" + fVar.name() + ";entries").c_str(),
                                              binning, universes));
      }

      virtual ~CrossSection() = default;

      virtual void mc(const evt::CVUniverse& event) override
      {
        fEfficiencyNum->Fill(&event, fVar.truth(event), event.GetWeight());
        fMigration->Fill(&event, fVar.truth(event), fVar.reco(event), event.GetWeight());
      }

      virtual void truth(const evt::CVUniverse& event) override
      {
        fEfficiencyDenom->Fill(&event, fVar.truth(event), event.GetWeight());
      }

      virtual void data(const evt::CVUniverse& event) override
      {
        fSignalEvents->Fill(&event, fVar.reco(event), event.GetWeight());
      }

    private:
      VARIABLE fVar;  //VARIABLE in which a differential cross section will be extracted

      //Signal histograms needed to extract a cross section
      std::unique_ptr<MIGRATION> fMigration;
      std::unique_ptr<HIST> fSignalEvents; //TODO: This just needs to be a TH1D!  It only comes from data.
      std::unique_ptr<HIST> fEfficiencyNum;
      std::unique_ptr<HIST> fEfficiencyDenom;
  };
}
