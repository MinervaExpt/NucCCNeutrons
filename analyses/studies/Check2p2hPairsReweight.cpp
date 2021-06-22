//File: Check2p2hPairsReweight.cpp
//Brief: Plots 3 physics quantities for validating New Systematics Framework
//       systematic universe usage.  Run over CCQENu with an empty signal definition
//       and use the results with Rob's validation suite in PlotUtils.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/Cut.h" //For PlotUtils::detail::empty

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/Check2p2hPairsReweight.h"

//evt includes
#include "evt/Universe.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  std::vector<util::NamedCategory<int>> nucleonCategories{{{0}, "nn"}, {{1}, "np"}, {{2}, "pp"}};
}

namespace ana
{
  Check2p2hPairsReweight::Check2p2hPairsReweight(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                               std::map<std::string, std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                               fEAvailSmearingByNucleonPair(::nucleonCategories, dir, "EAvailSmearing", "E_{available, true};E_{available, reco}", config["EAvailBins"].as<std::vector<double>>(), config["EAvailBins"].as<std::vector<double>>(), univs), fVariable(config["EAvail"]),
                               fQ0Q3ByNucleonPair(::nucleonCategories, dir, "q0_q3", "q_{0, true};q_{3, true}", config["q3Bins"].as<std::vector<double>>(), config["q0Bins"].as<std::vector<double>>(), univs)
  {
    fTargetNucleons = dir.make<PlotUtils::HistWrapper<evt::Universe>>("TargetNucleons", "Target Nucleons for 2p2h Interactions;PDG Code", 3, 0, 3, univs);
    fEAvailSmearingAll2p2h = dir.make<units::WithUnits<PlotUtils::Hist2DWrapper<evt::Universe>, MeV, MeV, events>>("EAvailSmearing", "2p2h E_{available} Smearing;E_{available, true};E_{available, reco}", config["EAvailBins"].as<std::vector<double>>(), config["EAvailBins"].as<std::vector<double>>(), univs);
    fQ0Q3Overall = dir.make<HIST2D<GeV, GeV>>("q0_q3_allEvents", "All Events Phase Space;q_{3, true};q_{0, true}", config["q3Bins"].as<std::vector<double>>(), config["q0Bins"].as<std::vector<double>>(), univs);
  }

  void Check2p2hPairsReweight::mcSignal(const evt::Universe& event, const events weight)
  {
    const auto q0 = event.GetTruthQ0(), q3 = event.GetTruthQ3();
    fQ0Q3Overall->Fill(&event, q3, q0, weight);

    //Only plot target nucleon pairs for 2p2h interactions
    if(event.GetInt("mc_intType") == 8 && event.GetInt("mc_targetZ") > 1)
    {
      const int nProtons = event.GetInt("mc_targetNucleon") - 2000000200;
      fTargetNucleons->FillUniverse(event, nProtons, weight.in<events>());
      fEAvailSmearingByNucleonPair[nProtons].Fill(&event, fVariable.truth(event), fVariable.reco(event), weight);
      fEAvailSmearingAll2p2h->Fill(&event, fVariable.truth(event), fVariable.reco(event), weight);
      fQ0Q3ByNucleonPair[nProtons].Fill(&event, q3, q0, weight);
    }
  }

  void Check2p2hPairsReweight::afterAllFiles(const events /*passedSelection*/)
  {
    fTargetNucleons->SyncCVHistos();
  }
}

namespace
{
  static ana::Study::Registrar<ana::Check2p2hPairsReweight> Check2p2hPairsReweight_reg("Check2p2hPairsReweight");
}
