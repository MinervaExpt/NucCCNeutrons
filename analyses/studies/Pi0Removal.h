//File: Pi0Removal.h
//Brief: This Study quantifies how well I differentiate GENIE-neutron-induced candidates from pi0-induced candidates.
//       It includes branches dedicated to pi0 removal.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"
#include "util/units.h"
#include "util/mathWithUnits.h"

//c++ includes
#include <fstream>

#ifndef ANA_PI0REMOVAL_H
#define ANA_PI0REMOVAL_H

DECLARE_UNIT_WITH_TYPE_AND_YAML(Clusters, int)
DECLARE_UNIT_WITH_TYPE_AND_YAML(Digits, int)

namespace ana
{
  class Pi0Removal: public Study
  {
    public:
      Pi0Removal(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~Pi0Removal() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      //Normalize and syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      //Cuts that decide whether a Candidate or FSPart should be counted
      ana::NeutronMultiplicity fCuts;

      //Format for neutron candidate information.
      /*struct NeutronCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
        int nClusters;
      };*/ //I'd need this if I ever had something to compare to data

      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
        /*int nClusters;
        int nDigits;
        MeV highestDigitE;*/
        int FS_index; //Mapping from a Candidate to an FSPart by index in the array of FSParts
        mm dist_to_edep_as_neutron; //Distance parent and ancestors travelled that were neutrons
        int nViews; //Candidates with >= 2 views may have some 3D reconstruction information
        double cosine_to_muon; //angle between the fit direction of a 3D neutron candidate and
                             //the direction from its' start point back to the muon.
        mm x3D; //X position from fit to 3D candidates
        mm y3D; //Y position from fit to 3D candidates
      };

      //Format for FS particle information.
      struct FSPart
      {
        int PDGCode;
        MeV energy;
        units::LorentzVector<MeV> momentum;
      };

      template <class XUNIT, class WUNIT>
      using HIST = units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, XUNIT, WUNIT>;

      util::Categorized<HIST<MeV, neutrons>, int> fPDGToBestInvMass; //The closest invariant mass to the pi0 mass among all other candidates
      util::Categorized<PlotUtils::HistWrapper<evt::Universe>, int> fPDGToCosineToMuon; //Are candidates that shoot off at angles not aligned with direction to muon
                                                                                        //primarily neutrons or pi0s?
  };
}

#endif //ANA_PI0REMOVAL_H
