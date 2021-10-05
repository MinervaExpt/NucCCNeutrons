//File: MoNAReweightValidation.h
//Brief: A Study to debug the NeutronInelasticReweight Model by reproducing the plots I extracted its ratios from.
//       Compare to figure 3 from https://www.osti.gov/pages/servlets/purl/1454859
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"
#include "util/units.h"

#ifndef ANA_CANDIDATECAUSES_H
#define ANA_CANDIDATECAUSES_H

namespace PlotUtils
{
  class TargetUtils;
}

namespace ana
{
  class MoNAReweightValidation: public Study
  {
    public:
      MoNAReweightValidation(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                             std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~MoNAReweightValidation() = default;

      void mcSignal(const evt::Universe& event, const events weight) override;
      void truth(const evt::Universe& event, const events weight) override;

      void afterAllFiles(const events /*passedSelection*/) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {};

    private:
      using Hist_t = units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, neutrons>;
      util::Categorized<Hist_t, std::multiset<int>> fTruthNeutronKEPerInteractionMode;
      util::Categorized<Hist_t, std::multiset<int>> fSelectedNeutronKEPerInteractionMode;
      Hist_t* fTruthTotalNumberOfNeutrons;

      PlotUtils::TargetUtils fFiducial;

      template <class FUNC>
      void loopAllNeutrons(const evt::Universe& univ, const FUNC&& func);
  };
}

#endif //ANA_CANDIDATECAUSES_H
