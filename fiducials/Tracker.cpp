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

namespace fid
{
  class Tracker: public Fiducial
  {
    public:
      Tracker(const YAML::Node& config): Fiducial(config)
      {
        const mm zMin = config["reco"]["min"].as<mm>(),
                 zMax = config["reco"]["max"].as<mm>(),
                 apothem = config["apothem"].as<mm>();

        PlotUtils::TargetUtils targetInfo;
        fNNucleons = targetInfo.GetTrackerNNucleons(zMin.in<mm>(), zMax.in<mm>(), false, apothem.in<mm>());

        recoCuts.push_back(new reco::Apothem(config, "Apothem"));
        recoCuts.push_back(new reco::IsInTarget(config, "Tracker"));

        phaseSpace.push_back(new truth::Apothem(config, "Apothem"));
        phaseSpace.push_back(new truth::IsInTarget(config, "Tracker"));
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
  static fid::Fiducial::Registrar<fid::Tracker> Tracker_reg("Tracker");
}
