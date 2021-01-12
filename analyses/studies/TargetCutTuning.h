//File: TargetCutTuning.h
//Brief: Study nuclear target event selections to choose optimal z position cut values.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"

#ifndef ANA_TARGETCUTTUNING_H
#define ANA_TARGETCUTTUNING_H

namespace ana
{
  class TargetCutTuning: public Study
  {
    public:
      TargetCutTuning(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~TargetCutTuning() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      //syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override;

      //Do nothing for Truth tree and data
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      util::Categorized<units::WithUnits<HistWrapper<evt::Universe>, mm, events>, background_t> fZPositionsByTarget; //Event z positions by truth target
      units::WithUnits<HistWrapper<evt::Universe>, mm, events>* fSelectedSignalZPositions;

      util::Categorized<util::Categorized<units::WithUnits<Hist2DWrapper<evt::Universe>, mm, mm, events>, int>, background_t> fXYPositionsByTargetByMaterial;
      util::Categorized<units::WithUnits<Hist2DWrapper<evt::Universe>, mm, mm, events>, int> fSelectedSignalXYPositionsByMaterial;
  };
}

#endif //ANA_TARGETCUTTUNING_H
