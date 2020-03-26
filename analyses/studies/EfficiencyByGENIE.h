//File: EfficiencyByGENIE.h
//Brief: Am I less efficient in a particular GENIE category?
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/MuonMomentum.cpp"

//util includes
#include "util/Categorized.h"

#ifndef ANA_EFFICIENCYBYGENIE_H
#define ANA_EFFICIENCYBYGENIE_H

namespace ana
{
  class EfficiencyByGENIE: public Study
  {
    public:
      EfficiencyByGENIE(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::CVUniverse*>>& universes);
      virtual ~EfficiencyByGENIE() = default;

      //Do this study only for MC signal and truth events
      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override;
      virtual void truth(const evt::CVUniverse& event, const events weight) override;

      //Normalize fPDGToObservables and syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::CVUniverse& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void data(const evt::CVUniverse& /*event*/) override {};

    private:
      //VARIABLES in whic I'll report efficiency
      ana::MuonPz fPz;
      ana::MuonPT fPT;

      using HIST = units::WithUnits<Hist2DWrapper<evt::CVUniverse>, GeV, GeV, events>;

      util::Categorized<HIST, int> fGENIEToEfficiencyNum; //Map generated interaction type to efficiency numerator
      util::Categorized<HIST, int> fGENIEToEfficiencyDenom; //Map generated interaction type to efficiency denominator
  };
}

#endif //ANA_EFFICIENCYBYGENIE_H
