//File: TargetCutTuning.cpp
//Brief: A study on which reconstructed positions are truly inside each nuclear target.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Cut includes
#include "cuts/truth/Cut.h"

//signal includes
#include "analyses/studies/TargetCutTuning.h"

//util includes
#include "util/Factory.cpp"

namespace ana
{
  TargetCutTuning::TargetCutTuning(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fZPositionsByTarget(backgrounds, dir, "VertexZ", "Reco Vertex Z", config["binning"]["nBins"].as<double>(), config["binning"]["min"].as<double>(), config["binning"]["max"].as<double>(), univs)
  {
    fSelectedSignalZPositions = dir.make<units::WithUnits<HistWrapper<evt::CVUniverse>, mm, events>>("VertexZ_SelectedSignal", "Reco Vertex Z", config["binning"]["nBins"].as<double>(), config["binning"]["min"].as<double>(), config["binning"]["max"].as<double>(), univs);
    //fSelectedZPositions = dir.make<units::WithUnits<HistWrapper<evt::CVUniverse>, mm, events>>("VertexZ_Selected", "Reco Vertex Z", config["binning"].as<std::vector<double>>(), univs);
  }

  void TargetCutTuning::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    fSelectedSignalZPositions->Fill(&event, event.GetVtx().z(), weight);
  }

  void TargetCutTuning::mcBackground(const evt::CVUniverse& event, const background_t& background, const events weight)
  {
    fZPositionsByTarget[background].Fill(&event, event.GetVtx().z(), weight);
  }

  /*void TargetCutTuning::data(const evt::CVUniverse& event, const events weight)
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
