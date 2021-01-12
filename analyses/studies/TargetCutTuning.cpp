//File: TargetCutTuning.cpp
//Brief: A study on which reconstructed positions are truly inside each nuclear target.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Cut includes
#include "cuts/truth/Cut.h"

//signal includes
#include "analyses/studies/TargetCutTuning.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  const std::map<int, std::string> elements = {{6, "Carbon"},
                                               {26, "Iron"},
                                               {82, "Lead"},
                                               {1, "Hydrogen"},
                                               {8, "Oxygen"},
                                               {2, "Helium"}};
}

namespace ana
{
  TargetCutTuning::TargetCutTuning(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                   std::map<std::string, std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                               fZPositionsByTarget(backgrounds, dir, "VertexZ", "Reco Vertex Z",
                                                                                                                   config["binning"]["z"]["nBins"].as<int>(),
                                                                                                                   config["binning"]["z"]["min"].as<double>(),
                                                                                                                   config["binning"]["z"]["max"].as<double>(), univs),
                                                                                               fXYPositionsByTargetByMaterial(backgrounds, dir,
                                                                                                                   "VertexXY", "Reco Vertex X; Reco Vertex Y",
                                                                                                                   ::elements, dir,
                                                                                                                   config["binning"]["xy"]["nBins"].as<int>(),
                                                                                                                   config["binning"]["xy"]["min"].as<double>(),
                                                                                                                   config["binning"]["xy"]["max"].as<double>(),
                                                                                                                   config["binning"]["xy"]["nBins"].as<int>(),
                                                                                                                   config["binning"]["xy"]["min"].as<double>(),
                                                                                                                   config["binning"]["xy"]["max"].as<double>(),
                                                                                                                   univs),
                                                                                               fSelectedSignalXYPositionsByMaterial("VertexXY_Signal",
                                                                                                                   "Reco Vertex X; Reco Vertex Y",
                                                                                                                   GENIECategories, dir,
                                                                                                                   config["binning"]["xy"]["nBins"].as<int>(),
                                                                                                                   config["binning"]["xy"]["min"].as<double>(),
                                                                                                                   config["binning"]["xy"]["max"].as<double>(),
                                                                                                                   config["binning"]["xy"]["nBins"].as<int>(),
                                                                                                                   config["binning"]["xy"]["min"].as<double>(),
                                                                                                                   config["binning"]["xy"]["max"].as<double>(),
                                                                                                                   univs)
  {
    fSelectedSignalZPositions = dir.make<units::WithUnits<HistWrapper<evt::Universe>, mm, events>>("VertexZ_Signal", "Reco Vertex Z",
                                                                                                   config["binning"]["z"]["nBins"].as<int>(),
                                                                                                   config["binning"]["z"]["min"].as<double>(),
                                                                                                   config["binning"]["z"]["max"].as<double>(), univs);
    //fSelectedZPositions = dir.make<units::WithUnits<HistWrapper<evt::Universe>, mm, events>>("VertexZ_Selected", "Reco Vertex Z", config["binning"].as<std::vector<double>>(), univs);
  }

  void TargetCutTuning::mcSignal(const evt::Universe& event, const events weight)
  {
    fSelectedSignalZPositions->Fill(&event, event.GetVtx().z(), weight);
    fSelectedSignalXYPositionsByMaterial[event.GetTruthTargetZ()].Fill(&event, event.GetVtx().x(), event.GetVtx().y(), weight);
  }

  void TargetCutTuning::mcBackground(const evt::Universe& event, const background_t& background, const events weight)
  {
    fZPositionsByTarget[background].Fill(&event, event.GetVtx().z(), weight);
    fXYPositionsByTargetByMaterial[background][event.GetTruthTargetZ()].Fill(&event, event.GetVtx().x(), event.GetVtx().y(), weight);
  }

  /*void TargetCutTuning::data(const evt::Universe& event, const events weight)
  {
    fSelectedZPositions->Fill(&event, event.GetVtx().z(), weight);
  }
  */

  void TargetCutTuning::afterAllFiles(const events /*passedAllSelections*/)
  {
    fZPositionsByTarget.visit([](auto& hist) { hist.SyncCVHistos(); });
    fSelectedSignalZPositions->SyncCVHistos();
    //fSelectedZPosition.SyncCVHistos();
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::TargetCutTuning> TargetCutTuning_reg("TargetCutTuning");
}
