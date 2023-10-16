//File: DataMCRatioReweighter.h
//Brief: A Reweighter that changes MnvTunev1 into NuWro's Spectral Function (SF)
//       model for initial state nucleon kinematics.  Original only designed for QE on Carbon.
//       Reweight developed by and based on code by Tejin Cai at https://cdcvs.fnal.gov/redmine/projects/minerva-sw/repository/entry/AnalysisFramework/Ana/CCQENu/ana_common/src/CCQENuUtils.cxx#L2753
//Author: Tejin Cai (original algorithm), Andrew Olivier aolivier@ur.rochester.edu

#ifndef DATAMCRATIOREWEIGHTER_CPP
#define DATAMCRATIOREWEIGHTER_CPP

//ROOT includes
#include "TFile.h"
#include "TH1D.h"

//Reweighter includes
#include "PlotUtils/Reweighter.h"

//studies includes
#include "analyses/studies/MuonMomentum.cpp"

//util includes
#include "util/Factory.cpp"
#include "util/units.h"

//evt includes
#include "evt/Universe.h"

//c++ includes
#include <memory>

template <class UNIVERSE, class EVENT = PlotUtils::detail::empty>
class DataMCRatioReweighter: public PlotUtils::Reweighter<UNIVERSE, EVENT>
{
  public:
    DataMCRatioReweighter(const YAML::Node& config): PlotUtils::Reweighter<UNIVERSE, EVENT>(), fVar(config["variable"])
    {
      constexpr auto histName = "backgroundSubtracted";

      const auto dataFileName = config["dataFile"].as<std::string>();
      std::unique_ptr<TFile> dataFile(TFile::Open(dataFileName.c_str()));
      if(!dataFile) throw std::runtime_error("Failed to open a data file at " + dataFileName + " for data/MC reweight.");
      if(!dynamic_cast<PlotUtils::MnvH1D*>(dataFile->Get(histName))) throw std::runtime_error("Failed to find a histogram named backgroundSubtracted in " + dataFileName);

      const auto mcFileName = config["mcFile"].as<std::string>();
      std::unique_ptr<TFile> mcFile(TFile::Open(mcFileName.c_str()));
      if(!mcFile) throw std::runtime_error("Failed to open an MC file at " + mcFileName + " for data/MC reweight.");
      if(!dynamic_cast<PlotUtils::MnvH1D*>(mcFile->Get(histName))) throw std::runtime_error("Failed to find a histogram named backgroundSubtracted in " + mcFileName);

      fWeightHist.reset(dynamic_cast<PlotUtils::MnvH1D*>(dataFile->Get(histName))->Clone());
      fWeightHist->Divide(fWeightHist.get(), dynamic_cast<PlotUtils::MnvH1D*>(mcFile->Get(histName)));
    }

    virtual ~DataMCRatioReweighter() = default;

    double GetWeight(const UNIVERSE& univ, const EVENT& /*event*/) const override
    {
      if(univ.IsTruth()) return 1; //Just like the MINOS reweighter, I can't run this reweighter on the Truth tree because it uses a reco variable.
      return fWeightHist->GetBinContent(fWeightHist->FindBin(fVar.reco(univ).template in<GeV>()));
    }

    std::string GetName() const override { return "DataMCRatio"; }
    bool DependsReco() const override { return false; }

  private:
    ana::MuonPT fVar; //It would be easy to change this out with one of my other "calculators" if someone needed to
    std::unique_ptr<TH1D> fWeightHist; //Ratio of background-subtracted data to background-subtracted MC
};

namespace
{
  plgn::Registrar<PlotUtils::Reweighter<evt::Universe, PlotUtils::detail::empty>, DataMCRatioReweighter<evt::Universe, PlotUtils::detail::empty>> reg_NuWroSFReweight("DataMCRatioReweighter");
}

#endif //DATAMCRATIOREWEIGHTER_CPP
