//File: CandidateCauses.h
//Brief: A Study to help understand the secondary particles that deposit the energy in neutron candidates.
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

namespace ana
{
  class CandidateCauses: public Study
  {
    public:
      CandidateCauses(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                      std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~CandidateCauses() = default;

      void mcSignal(const evt::Universe& event, const events weight) override;

      void afterAllFiles(const events /*passedSelection*/) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      ana::NeutronMultiplicity fCuts;

      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        int nCauses;
        int FS_index;
        mm dist_to_edep_as_neutron;
      };

      struct Cause
      {
        int pdgCode;
        MeV energy;
      };

      struct FSPart
      {
        int PDGCode;
        MeV energy;
      };

      PlotUtils::HistWrapper<evt::Universe>* fCauseNames;
      PlotUtils::Hist2DWrapper<evt::Universe>* fCauseNameVersusEDep;
      //TODO: Plot leading Cause and fraction of its energy over all Causes' energies
      util::Categorized<units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, events>, int> fPDGCodeToCauseEnergy;

      std::map<int, std::string> fPDGToName;
  };
}

#endif //ANA_CANDIDATECAUSES_H
