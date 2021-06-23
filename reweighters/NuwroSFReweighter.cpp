//File: NuWroSFReweighter.h
//Brief: A Reweighter that changes MnvTunev1 into NuWro's Spectral Function (SF)
//       model for initial state nucleon kinematics.  Original only designed for QE on Carbon.
//       Reweight developed by and based on code by Tejin Cai at https://cdcvs.fnal.gov/redmine/projects/minerva-sw/repository/entry/AnalysisFramework/Ana/CCQENu/ana_common/src/CCQENuUtils.cxx#L2753
//Author: Tejin Cai (original algorithm), Andrew Olivier aolivier@ur.rochester.edu

#ifndef NUWROSFREWEIGHTER_CPP
#define NUWROSFREWEIGHTER_CPP

//ROOT includes
#include "TFile.h"
#include "TH2D.h"

//Reweighter includes
#include "PlotUtils/reweighters/Reweighter.h"

//util includes
#include "util/Factory.cpp"
#include "util/units.h"

//evt includes
#include "evt/Universe.h"

//c++ includes
#include <memory>

template <class UNIVERSE, class EVENT = PlotUtils::detail::empty>
class NuWroSFReweighter: public PlotUtils::Reweighter<UNIVERSE, EVENT>
{
  public:
    NuWroSFReweighter(const YAML::Node& /*config*/): PlotUtils::Reweighter<UNIVERSE, EVENT>()
    {
      //TODO: If you need to change the reweight file name often for debugging,
      //      make it a parameter that optionally comes from config.
      const char* mParamLocation = std::getenv("MPARAMFILESROOT");
      if(!mParamLocation) throw std::runtime_error("MPARAMFILESROOT needs to be set to use NuWroSFReweighter, but it's not set.");

      std::unique_ptr<TFile> nuWroReweightFile(TFile::Open((std::string(mParamLocation) + "data/Reweight/q0q3ProtonWeight.root").c_str()));
      if(!nuWroReweightFile) throw std::runtime_error("Failed to find a file at MPARAMFILESROOT/data/Reweight/q0q3ProtonWeight.root"); //TODO: Doesn't ROOT print a warning about this automatically?

      fWeightHist = dynamic_cast<TH2D*>(nuWroReweightFile->Get("nuwroLFG_mnvGENIEv2_qsq_kF_qelike_qe_oth_ratio")->Clone());
    }

    virtual ~NuWroSFReweighter() = default;

    double GetWeight(const UNIVERSE& univ, const EVENT& /*event*/) const override
    {
      if(univ.GetInt("mc_intType") != 1 || univ.GetInt("mc_current") != 1) return 1.0; //not CCQE
      if(univ.GetInt("mc_targetZ") != 6) return 1.0; //Reweight files designed only for Carbon originally

      const units::LorentzVector<MeV> neutrino = univ.GetVecDouble("mc_incomingPartVec"),
                                      lepton = univ.GetVecDouble("mc_primFSLepton"),
                                      initNuc = univ.GetVecDouble("mc_initNucVec");

      const auto qSq = units::fabs((neutrino - lepton).mass()),
                 kf = initNuc.p().mag();
      const int qSqBin = fWeightHist->GetXaxis()->FindBin(qSq.in<GeV>()),
                kfBin = fWeightHist->GetYaxis()->FindBin(kf.in<GeV>());
      return fWeightHist->GetBinContent(qSqBin, kfBin);
    }

    std::string GetName() const override { return "NuWroSF"; }
    bool DependsReco() const override { return false; }

  private:
    TH2D* fWeightHist; //Contains reweight values produced by Tejin with dedicated NuWro and GENIE generator samples using NUISSANCE
};

namespace
{
  plgn::Registrar<PlotUtils::Reweighter<evt::Universe, PlotUtils::detail::empty>, NuWroSFReweighter<evt::Universe, PlotUtils::detail::empty>> reg_NuWroSFReweight("NuWroSFReweighter");
}

#endif //NUWROSFREWEIGHTER_CPP
