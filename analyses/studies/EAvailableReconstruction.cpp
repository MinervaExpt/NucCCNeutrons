//File: EAvailableReconstruction.cpp
//Brief: Plots 3 physics quantities for validating New Systematics Framework
//       systematic universe usage.  Run over CCQENu with an empty signal definition
//       and use the results with Rob's validation suite in PlotUtils.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/EAvailableReconstruction.h"
#include "analyses/studies/TruthInteractionCategories.h"

//evt includes
#include "evt/CVUniverse.h"
#include "evt/arachne.h"
#include "evt/EventID.h"

//util includes
#include "util/Factory.cpp"

namespace ana
{
  EAvailableReconstruction::EAvailableReconstruction(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                                     std::vector<background_t>& backgrounds,
                                                     std::map<std::string, std::vector<evt::CVUniverse*>>& univs)
                                                    :Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                     fNNeutrons(config["multiplicity"]),
                                                     fEAvailable(config["EAvailable"]),
                                                     fEAvailableMax(config["EAvailable"]["Max"].as<GeV>()),
                                                     fMismatchedCutLinks("EAvailableCutsMismatched.txt"),
                                                     fNoRecoLinks("EAvailableNoReco.txt"),
                                                     fTruthMultiplicity(pionFSCategories, dir, "TruthNeutronMultiplicity", "Truth", std::vector<double>{0, 1, 2, 4, 10}, univs),
                                                     fEAvailableResidual(pionFSCategories, dir,  "EAvailableResidual", "E_{Available} Residual;#frac{Reco - Truth}{Truth}", 100, -1, 4, univs)
  {
    fTruthAvailWhenNoReco = dir.make<HIST<MeV>>("TruthEAvailWhenNoReco", "Truth E_{available}", 100, 0, 2000, univs);
  }

  void EAvailableReconstruction::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    const GeV EAvailReco = fEAvailable.reco(event),
              EAvailTruth = fEAvailable.truth(event);

    const unitless EAvailResidual = (EAvailReco - EAvailTruth).in<GeV>()/EAvailTruth.in<GeV>();

    if((EAvailReco <= fEAvailableMax) && (EAvailTruth > fEAvailableMax))
    {
      const auto found = std::find_if(ana::pionFSCategories.begin(), ana::pionFSCategories.end(),
                                      [&event](const auto category)
                                      { return (*category)(event); });
      const auto whichCategory = (found != ana::pionFSCategories.end())?*found:nullptr;
  
      fTruthMultiplicity[whichCategory].Fill(&event, fNNeutrons.truth(event), weight);
  
      if(EAvailTruth != 0_GeV) fEAvailableResidual[whichCategory].Fill(&event, EAvailResidual, weight);

      fMismatchedCutLinks << util::arachne(event.GetEventID(false), false);
    }

    if(fabs(EAvailResidual + 1_unitless) < 1e-3_unitless)
    {
      fTruthAvailWhenNoReco->Fill(&event, EAvailTruth);
      fNoRecoLinks << util::arachne(event.GetEventID(false), false);
    }
  }

  void EAvailableReconstruction::afterAllFiles(const events /*passedSelection*/)
  {
    fTruthMultiplicity.visit([](auto& hist) { hist.SyncCVHistos(); });
    fEAvailableResidual.visit([](auto& hist) { hist.SyncCVHistos(); });
  }
}

namespace
{
  static ana::Study::Registrar<ana::EAvailableReconstruction> EAvailableReconstruction_reg("EAvailableReconstruction");
}
