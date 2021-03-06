//File: PerCandidateTree.h
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"
#include "util/units.h"
#include "util/mathWithUnits.h"

#ifndef SIG_NEUTRONDETECTION_H
#define SIG_NEUTRONDETECTION_H

namespace ana
{
  class PerCandidateTree: public Study
  {
    public:
      PerCandidateTree(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~PerCandidateTree() = default;

      //Do this study only for MC signal and background events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;
      void mcBackground(const evt::Universe& event, const background_t& background, const events weight) override;

      //No histograms to normalize
      virtual void afterAllFiles(const events /*passedSelection*/) override {};

      //Do nothing for the Truth tree and data
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      //Cuts that decide whether a Candidate or FSPart should be counted
      ana::NeutronMultiplicity fCuts;

      //Format for neutron candidate information.
      struct NeutronCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
      };

      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        mm muon_long;
        mm muon_transverse;
        ns time;
        int FS_index; //Mapping from a Candidate to an FSPart by index in the array of FSParts
        mm dist_to_edep_as_neutron; //Distance parent and ancestors travelled that were neutrons
        int nDigits;
        int nClusters;
        MeV highestDigitE;
      };

      //Format for FS particle information.
      struct FSPart
      {
        int PDGCode;
        MeV energy;
        double angle_wrt_z; //Angle w.r.t. the z axis of the detector in radians
      };

      //These functions do the real work so I can reuse the same code in multiple cases
      void fillMCBranches(const MCCandidate& cand, const std::vector<FSPart>& fs, const units::LorentzVector<mm> vertex);
      void fillRecoBranches(const std::vector<MCCandidate>::const_iterator cand, const units::LorentzVector<mm> vertex, const std::vector<MCCandidate>& allCands);
      void connectMCBranches(TTree& tree);
      void connectRecoBranches(TTree& tree);

      //Tree and branches filled for each candidate
      TTree* fMCTree;
      util::Categorized<TTree, background_t> fBackgroundTrees;

      //Truth-matched branches
      double fFSPDG;
      double fTruthE;
      double fTruthAngle;

      //Reco branches
      double fEDep;
      double fDistFromVertex;
      double fDeltaT;
      double fAngleWrtZ;
      double fAngleTransverseToZ;
      double fCosineWrtMuon;
      double fSineWrtMuon;
      int fNDigits;
      int fNClusters;
      int fNCandidates;
      double fHighestDigitE;
      double fSmallestAngleDiff;

      //Per-event weight
      double fEventWeight;

      double angle(const MCCandidate& cand, const units::LorentzVector<mm>& vertex) const;
  };
}

#endif //SIG_NEUTRONDETECTION_H
