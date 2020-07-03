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
                       std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::CVUniverse*>>& universes);
      virtual ~TargetCutTuning() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override;

      //syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      virtual void mcBackground(const evt::CVUniverse& /*event*/, const background_t& /*background*/, const events /*weight*/) override;

      //Do nothing for Truth tree and data
      virtual void truth(const evt::CVUniverse& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::CVUniverse& /*event*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      util::Categorized<units::WithUnits<HistWrapper<evt::CVUniverse>, mm, events>, background_t> fZPositionsByTarget; //Event z positions by truth target

      units::WithUnits<HistWrapper<evt::CVUniverse>, mm, events>* fSelectedSignalZPositions;

      //units::WithUnits<HistWrapper<evt::CVUniverse>, mm, events>* fSelectedZPositions; //Event z positions in data.  Compare to stack of fZPositionsByTarget.
  };
}

#endif //ANA_TARGETCUTTUNING_H
