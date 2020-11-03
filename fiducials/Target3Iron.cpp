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

namespace fid
{
  class Target3Iron: public Fiducial
  {
    static constexpr int targetZ = 26;

    public:
      Target3Iron(const YAML::Node& config): Fiducial(config)
      {
        const mm apothem = config["apothem"]["apothem"].as<mm>();

        PlotUtils::TargetUtils targetInfo;
        fNNucleons = targetInfo.GetPassiveTargetNNucleons(3, targetZ, false, apothem.in<mm>());

        recoCuts.push_back(new reco::Apothem(config["apothem"], "Apothem"));
        recoCuts.push_back(new reco::IsInTarget(config["zRange"], "Target3"));
        recoCuts.push_back(new reco::ThreeSectionTarget<targetZ>(config["zRange"], "Iron"));

        phaseSpace.push_back(new truth::Apothem(config["apothem"], "Apothem"));
        signalDef.push_back(new truth::IsInTarget(config["zRange"], "Target3"));
        phaseSpace.push_back(new truth::ThreeSectionTarget<targetZ>(config["zRange"], "Iron"));
      }

      virtual double NNucleons() const override
      {
        return fNNucleons;
      }

    private:
      double fNNucleons;
  };
}

namespace
{
  static fid::Fiducial::Registrar<fid::Target3Iron> Target3Iron_reg("Target3Iron");
}
