//File: Tracker.h
//Brief: A Tracker Fiducial is between MINERvA's nuclear target region
//       and the downstream ECAL.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//NucCCNeutrons includes
#include "cuts/reco/Apothem.h"
#include "cuts/reco/targets/IsInTarget.h"
#include "cuts/truth/Apothem.h"
#include "cuts/truth/targets/IsInTarget.h"
#include "fiducials/Fiducial.h"

//PlotUtils includes
#include "PlotUtils/TargetUtils.h"
#include "PlotUtils/TargetMassSystematics.h"
#include "PlotUtils/HistWrapper.h"

namespace fid
{
  class Tracker: public Fiducial
  {
    public:
      Tracker(const YAML::Node& config): Fiducial(config), fRecoZMin(config["zRange"]["reco"]["min"].as<mm>()),
                                                           fRecoZMax(config["zRange"]["reco"]["max"].as<mm>()),
                                                           fTruthZMin(config["zRange"]["truth"]["min"].as<mm>()),
                                                           fTruthZMax(config["zRange"]["truth"]["max"].as<mm>()),
                                                           fApothem(config["apothem"]["apothem"].as<mm>())
      {
        recoCuts.push_back(new reco::Apothem(config["apothem"], "Apothem"));
        recoCuts.push_back(new reco::IsInTarget(config["zRange"], "Tracker"));

        phaseSpace.push_back(new truth::Apothem(config["apothem"], "Apothem"));
        phaseSpace.push_back(new truth::IsInTarget(config["zRange"], "Tracker"));
      }

      virtual PlotUtils::MnvH1D* NNucleons(const bool isMC, std::map<std::string, std::vector<evt::Universe*>>& universes) const override
      {
        PlotUtils::TargetUtils targetInfo;
        const double nNucleons = targetInfo.GetTrackerNNucleons((isMC?fTruthZMin:fRecoZMin).in<mm>(), (isMC?fTruthZMax:fRecoZMax).in<mm>(), isMC, fApothem.in<mm>());
        PlotUtils::HistWrapper<evt::Universe> dummyHist("dummy", "dummy", 1, 0., 1., universes);
        //auto dummyHist = new PlotUtils::MnvH1D("dummy", "dummy", 1, 0., 1.);
        return PlotUtils::GetNTargetsScintillatorHist(nNucleons, dummyHist.hist);
      }

    private:
      mm fRecoZMin;
      mm fRecoZMax;
      mm fTruthZMin;
      mm fTruthZMax;
      mm fApothem;
  };
}

namespace
{
  static fid::Fiducial::Registrar<fid::Tracker> Tracker_reg("Tracker");
}
