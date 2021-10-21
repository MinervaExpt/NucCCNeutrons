//File: TejinSensitivity.h
//Brief: A Study to quantify Tejin's thesis's sensitivity to new systematics I develop.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"
#include "util/units.h"

#ifndef ANA_TEJINSENSITIVITY_H
#define ANA_TEJINSENSITIVITY_H

namespace ana
{
  class TejinSensitivity: public Study
  {
    public:
      TejinSensitivity(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                      std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~TejinSensitivity() = default;

      void mcSignal(const evt::Universe& event, const events weight) override;
      void data(const evt::Universe& /*event*/, const events /*weight*/) override;

      void afterAllFiles(const events /*passedSelection*/) override;

      //Do nothing for backgrounds and the Truth tree
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      ana::NeutronMultiplicity fCuts;

      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        int FS_index;
        mm dist_to_edep_as_neutron;
        int nViews;
      };

      struct RecoCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        int nViews;
      };

      /*struct Cause
      {
        int pdgCode;
        MeV energy;
      };*/

      struct FSPart
      {
        int PDGCode;
        MeV energy;
        units::LorentzVector<MeV> momentum;
      };

      using Hist_t = units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, neutrons>;
      Hist_t* fLeadingCandidateEDep;
      Hist_t* fLeadingNeutronKE;
      Hist_t* fLeadingCandidateParentKE;
  };
}

#endif //ANA_TEJINSENSITIVITY_H
