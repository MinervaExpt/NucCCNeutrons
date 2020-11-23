//File: Target3Carbon.h
//Brief: A Target3Carbon Fiducial is between MINERvA's nuclear target region
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
  class Target3Carbon: public Fiducial
  {
    static constexpr int targetZ = 6;

    public:
      Target3Carbon(const YAML::Node& config): Fiducial(config), fApothem(config["apothem"]["apothem"].as<mm>())
      {
        recoCuts.push_back(new reco::Apothem(config["apothem"], "Apothem"));
        recoCuts.push_back(new reco::IsInTarget(config["zRange"], "Target3"));
        recoCuts.push_back(new reco::ThreeSectionTarget<targetZ>(config["zRange"], "Carbon"));

        phaseSpace.push_back(new truth::Apothem(config["apothem"], "Apothem"));
        signalDef.push_back(new truth::IsInTarget(config["zRange"], "Target3"));
        phaseSpace.push_back(new truth::ThreeSectionTarget<targetZ>(config["zRange"], "Carbon"));
      }

      virtual double NNucleons(const bool isMC) const override
      {
        PlotUtils::TargetUtils targetInfo;
        return targetInfo.GetPassiveTargetNNucleons(3, targetZ, isMC, fApothem.in<mm>());
      }

    private:
      mm fApothem;
  };
}

namespace
{
  static fid::Fiducial::Registrar<fid::Target3Carbon> Target3Carbon_reg("Target3Carbon");
}
