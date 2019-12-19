//File: CrossSection.h
//Brief: A CrossSection is an Analysis template that fills the histograms to extract a
//       differential cross section in a particular VARIABLE.  A VARIABLE has a name and
//       can be calculated from a CVUniverse both in truth() and in reco() in the same
//       units.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef ANA_ANALYSIS_H
#define ANA_ANALYSIS_H

//analysis includes
#include "analysis/Analysis.h"

//evt includes
#include "evt/CVUniverse.h"

//geo includes
#include "geo/Target.h"

//util includes
#include "util/Categorized.h"
#include "util/Directory.h"

namespace ana
{
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match

  template <class VARIABLE>
  class CrossSection: public Analysis
  {
    private:
      using UNIT = decltype(fVar.reco(std::declval<evt::CVUniverse>()));
      static_assert(std::is_same<UNIT, decltype(fVar.truth(std::declval<evt::CVUniverse>()))>::value,
                    "Reco and truth variable calculations must be in the same units!");

      using HIST = HistWrapper<WithUnits<MnvH1D, UNIT, events>>;
      using MIGRATION + HistWrapper2D<WithUnits<MnvH2D, UNIT, UNIT, events>>;

    public:
      CrossSection(const YAML::Node& config, util::Directory& parent,
                   const std::vector<geo::Target>& truthTargets): fVar(config["variable"])
      {
        auto dir = parent.mkdir(fVar.name());

        const auto binning = config["binning"].as<std::vector<double>>();
        fTruthTargetToSideband = util::Categorized<geo::Target*, Sideband>(truthTargets, dir, binning.size() - 1, binning.data());
        fMaterialToSideband = util::Categorized<int, HIST>(config["materials"].as<std::map<int, std::string>>(),
                                                           dir, binning.size() - 1, binning.data());
        fMigration = dir.make<MIGRATION>("Migration", ("Migration;Reco " + fVar.name() + ";True " + fVar.name() + ";entries").c_str(),
                                         binning.size() - 1, binning.data(), binning.size() - 1, binning.data());
        fSignalEvents = dir.make<HIST>("Signal", ("Signal;" + fVar.name() + ";entries").c_str(),
                                       binning.size() - 1, binning.data());
        fEfficiencyNum = dir.make<HIST>("EfficiencyNumerator", ("Efficiency Numerator;" + fVar.name() + ";entries").c_str(),
                                        binning.size() - 1, binning.data());
        fEfficiencyDenom = dir.make<HIST>("EfficiencyDenominator", ("Efficiency Denominator;" + fVar.name() + ";entries").c_str(),
                                          binning.size() - 1, binning.data());
      }

      virtual ~CrossSection() = default;

      //Fill signal-only plots for truth, reco, and MC which has both
      virtual void truthSignal(const evt::CVUniverse& event) override //Efficiency denominator
      {
        fEfficiencyDenom.univHist(&event)->Fill(fVar.truth(event));
      }

      virtual void dataSignal(const evt::CVUniverse& event) override //Selected events
      {
        fSignalEvents.univHist(&event)->Fill(fVar.reco(event));
      }

      virtual void MCSignal(const evt::CVUniverse& event) override //Efficiency numerator and migration matrices
      {
        fEfficiencyNum.univHist(&event)->Fill(fVar.truth(event));
        fMigration.univHist(&event)->Fill(fVar.truth(event), fVar.reco(event));
      }

      //Plots for plastic sideband constraint
      virtual void PlasticBackground(const evt::CVUniverse& event, const geo::Target* truthTarget) override
      {
        const auto found = fTruthTargetToSideband.find(truthTarget);
        if(found == fTruthTargetToSideband.end())
        {
          found = fTruthTargetToSideband.insert(std::make_pair(truthTarget, Sideband(truthTarget)));
        }

        found->Background.univHist(&event)->Fill(fVar.reco(event));
      }

      virtual void PlasticSideband(const evt::CVUniverse& event, const geo::Target* truthTarget) override
      {
        fTruthTargetToSideband[truthTarget].Sideband.univHist(&event)->Fill(fVar.reco(event));
      }

      //Plots for wrong material sideband constraint.  Remember that I can just
      //use the efficiency numerators and WrongMaterialBackground()s from the
      //other sections in this target as the signal model in this sideband.
      virtual void WrongMaterialBackground(const evt::CVUniverse& event) override
      {
        fMaterialToSideband[event.Getmc_targetZ()].univHist(&event).Fill(fVar.reco(event));
      }

      //TODO: I need some opportunity to SyncCVHistos() before these HISTs are written to a TFile.
      //      This has to happen before the destructor of the TFile from which util::Directory was made,
      //      so I don't think this class's destructor is the best way to handle this situation.

    private:
      VARIABLE fVar; //truth and reco algorithms for calculating the variable to be plotted

      struct Sideband
      {
        Sideband(const std::string& name, const std::string& title, int nBins, double* bins): Background((name + "PlasticBackground").c_str(), (title + " Plastic Background;" + fVar.name() + ";Entries").c_str(), nBins, bins),
                                                                                              Sideband((name + "PlasticSideband").c_str(), (title + " Plastic Sideband;" + fVar.name() + ";Entries").c_str(), nBins, bins)
        {
        }

        HIST Background;
        HIST Sideband;
      };

      util::Categorized<geo::Target*, Sideband> fTruthTargetToSideband; //Mapping from truth Target to sideband plots
      util::Categorized<int, HIST> fMaterialToSideband; //The wrong material sideband.  Materials are specified by Z.
      HistWrapper2D<MIGRATION>* fMigration;
      HIST* fSignalEvents;
      HIST* fEfficiencyNum;
      HIST* fEfficiencyDenom;
  };
}

#endif //ANA_ANALYSIS_H
