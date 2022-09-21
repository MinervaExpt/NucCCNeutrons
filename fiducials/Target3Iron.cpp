//File: Target3Iron.h
//Brief: A Target3Iron Fiducial is between MINERvA's nuclear target region
//       and the downstream ECAL.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//NucCCNeutrons includes
#include "cuts/reco/Apothem.h"
#include "cuts/reco/targets/IsInTarget.h"
#include "cuts/reco/targets/ThreeSectionTarget.h"
#include "cuts/truth/Apothem.h"
#include "cuts/truth/targets/IsInTarget.h"
#include "cuts/truth/targets/ThreeSectionTarget.h"

#include "fiducials/Fiducial.h"

//PlotUtils includes
#include "PlotUtils/TargetUtils.h"
#include "PlotUtils/TargetMassSystematics.h"
#include "PlotUtils/HistWrapper.h"

namespace fid
{
  class Target3Iron: public Fiducial
  {
    static constexpr int targetZ = 26;

    public:
      Target3Iron(const YAML::Node& config): Fiducial(config), fApothem(config["apothem"]["apothem"].as<mm>())
      {
        recoCuts.push_back(new reco::Apothem(config["apothem"], "Apothem"));
        recoCuts.push_back(new reco::IsInTarget(config["zRange"], "Target3"));
        recoCuts.push_back(new reco::ThreeSectionTarget<targetZ>(config["zRange"], "Iron"));

        phaseSpace.push_back(new truth::Apothem(config["apothem"], "Apothem"));
        signalDef.push_back(new truth::IsInTarget(config["zRange"], "Target3"));
        phaseSpace.push_back(new truth::ThreeSectionTarget<targetZ>(config["zRange"], "Iron"));
      }

      virtual PlotUtils::MnvH1D* NNucleons(const bool isMC, std::map<std::string, std::vector<evt::Universe*>>& universes) const override
      {
        PlotUtils::TargetUtils targetInfo;
        const double nNucleons = targetInfo.GetPassiveTargetNNucleons(3, targetZ, isMC, fApothem.in<mm>());
        //auto dummyHist = new PlotUtils::MnvH1D("dummy", "dummy", 1, 0., 1.);
        PlotUtils::HistWrapper<evt::Universe> dummyHist("dummy", "dummy", 1, 0., 1., universes);
        return PlotUtils::GetNTargetsIronHist(nNucleons, dummyHist.hist);
      }

    private:
      mm fApothem;
  };
}

namespace
{
  static fid::Fiducial::Registrar<fid::Target3Iron> Target3Iron_reg("Target3Iron");
}
