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
      Target3Carbon(const YAML::Node& config): Fiducial(config)
      {
        const mm apothem = config["apothem"].as<mm>();

        PlotUtils::TargetUtils targetInfo;
        fNNucleons = targetInfo.GetPassiveTargetNNucleons(3, targetZ, false, apothem.in<mm>());

        recoCuts.push_back(new reco::Apothem(config, "Apothem"));
        recoCuts.push_back(new reco::IsInTarget(config, "Target3"));
        recoCuts.push_back(new reco::ThreeSectionTarget<targetZ>(config, "Carbon"));

        phaseSpace.push_back(new truth::Apothem(config, "Apothem"));
        signalDef.push_back(new truth::IsInTarget(config, "Target3"));
        phaseSpace.push_back(new truth::ThreeSectionTarget<targetZ>(config, "Carbon"));
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
  static fid::Fiducial::Registrar<fid::Target3Carbon> Target3Carbon_reg("Target3Carbon");
}
