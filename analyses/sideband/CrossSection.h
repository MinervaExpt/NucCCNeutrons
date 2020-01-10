//File: CrossSection.h
//Brief: The sideband plots needed to extract a cross section in a single
//       VARIABLE (TODO: in multiple dimensions).  A sideband is defined by
//       1 or more reconstruction cuts that an event fails and can be delimited
//       by that event passes a set of selection cuts.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//sideband includes
#include "analyses/sideband/Sideband.h"

namespace side
{
  //This is the same concept of VARIABLE as used in analyses/signal/CrossSection.h and
  //all other CrossSeciton headers.
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class CrossSection: public Sideband
  {
    //Sanity checks on VARIABLE
    private:
      using UNIT = decltype(fVar.reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(fVar.truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = HistWrapper<WithUnits<MnvH1D, UNIT, events>>;

    public:
      //TODO: Categorized needs to work with Directory
      Sideband(util::Directory& dir, cuts_t&& passes,
               const backgrounds_t& backgrounds, const YAML::Node& config): Sideband(dir, passes, backgrounds, config),
                                                                            fVar("variable"),
                                                                            fBackgrounds(dir, backgrounds, "Background", "Reco " + fVar.name(),
                                                                                         config["binning"].as<std::vector<double>>().size() - 1,
                                                                                         config["binning"].as<std::vector<double>>().data())
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?

        fData = dir.make<HIST>("Data", ("Data;Reco " + fVar.name() + ";entries").c_str(), binning.size() - 1, binning.data());
        fSignal = dir.make<HIST>("TruthSignal", ("Truth Signal;Reco " + fVar.name() + ";entries").c_str(), binning.size() - 1, binning.data());
      }

      virtual ~CrossSection() = default;

      virtual void data(const CVUniverse& event) override
      {
        fData.univHist(&event).Fill(fVar.reco(event), event.GetWeight());
      }

      virtual void truthSignal(const CVUniverse& event) override
      {
        fSignal.univHist(&event).Fill(fVar.reco(event), event.GetWeight());
      }

      virtual void truthBackground(const CVUnvierse& event, const backgrounds_t::iterator background) override
      {
        fBackgrounds[background].univHist(&event).Fill(fVar.reco(event), event.GetWeight());
      }

    private:
      VARIABLE fVar;

      //Histograms for cross section extraction
      //These plots are all used together, and one of them is filled with data.
      //So, they're all in reco variables.
      HIST fData;
      HIST fSignal;
      util::Categorized<HIST, backgrounds_t::iterator> fBackgrounds;
  };
}
