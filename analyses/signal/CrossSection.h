//File: CrossSection.h
//Brief: A Signal template that just produces the plots you need
//       to extract a differential cross section in 1D (TODO in an arbitrary
//       number of dimensions).  Use it to implement a CrossSection
//       for your VARIABLE of interest.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/signal/Signal.h"

namespace evt
{
  class CVUniverse;
}

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
      using UNIT = decltype(fVar.reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(fVar.truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = HistWrapper<WithUnits<MnvH1D, UNIT, events>>;
      using MIGRATION = HistWrapper2D<WithUnits<MnvH2D, UNIT, UNIT, events>>;

    public:
      CrossSection(util::Directry& dir, const YAML::Node& config): Signal(dir, config), fVar("variable")
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?

        fMigration = dir.make<MIGRATION>("Migration", ("Migration;Reco " + fVar.name() + ";Truth " + fVar.name() + ";entries").c_str(),
                                         binning.size() - 1, binning.data(), binning.size() - 1, binning.data());
        fSignalEvents = dir.make<HIST>("Signal", ("Signal;" + fVar.name() + ";entries").c_str(),
                                       binning.size() - 1, binning.data());
        fEfficiencyNum = dir.make<HIST>("EfficiencyNumerator", ("Efficiency Numerator;" + fVar.name() + ";entries").c_str(),
                                        binning.size() - 1, binning.data());
        fEfficiencyDenom = dir.make<HIST>("EfficiencyDenominator", ("Efficiency Denominator;" + fVar.name() + ";entries").c_str(),
                                          binning.size() - 1, binning.data());
      }

      virtual ~CrossSection() = default;

      virtual void mc(const CVUniverse& event) override
      {
        fEfficiencyNum.univHist(&event)->Fill(fVar.truth(event));
        fMigration.univHist(&event)->Fill(fVar.truth(event), fVar.reco(event));
      }

      virtual void truth(const CVUniverse& event) override
      {
        fEfficiencyDenom.univHist(&event)->Fill(fVar.truth(event));
      }

      virtual void data(const CVUniverse& event) override
      {
        fSignalEvents.univHist(&event)->Fill(fVar.reco(event));
      }

    private:
      VARIABLE fVar;  //VARIABLE in which a differential cross section will be extracted

      //Signal histograms needed to extract a cross section
      MIGRATION fMigration;
      HIST fSignalEvents;
      HIST fEfficiencyNum;
      HIST fEfficiencyDenom;
  };
}
