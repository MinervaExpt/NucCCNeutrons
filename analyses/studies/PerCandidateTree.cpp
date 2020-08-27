//File: PerCandidateTree.cpp
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/PerCandidateTree.h"

//util includes
#include "util/Factory.cpp"

using namespace units;

namespace ana
{
  PerCandidateTree::PerCandidateTree(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fCuts(config["variable"])
  {
    fMCTree = dir.make<TTree>("MCPerCandidateTree", "One entry for each truth-matched neutron candidate");

    fMCTree->Branch("FSPDG", &fFSPDG);
    fMCTree->Branch("TruthE", &fTruthE);
    fMCTree->Branch("TruthAngle", &fTruthAngle);

    fMCTree->Branch("EDep", &fEDep);
    fMCTree->Branch("DistFromVertex", &fDistFromVertex);
    fMCTree->Branch("DeltaT", &fDeltaT);
    fMCTree->Branch("AngleWrtVertex", &fAngleWrtVertex);
    fMCTree->Branch("NDigits", &fNDigits);
    fMCTree->Branch("HighestDigitE", &fHighestDigitE);
    fMCTree->Branch("SmallestAngleDiff", &fSmallestAngleDiff);

    fMCTree->Branch("EventWeight", &fEventWeight);
  }

  void PerCandidateTree::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    fEventWeight = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(),
                                              event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron(),
                                              event.Getblob_n_digits(), event.Getblob_highest_digit_E());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z());
    const auto vertex = event.GetVtx();

    for(auto whichCand = cands.begin(); whichCand != cands.end(); ++whichCand)
    {
      const auto& cand = *whichCand;

      if(fCuts.countAsReco(cand, vertex))
      {
        int pdg = -1; //-1 represents "Other".  Thsi happens when I couldn't find a parent PDG code.
                      //I think that means either overlay or cross-talk.

        double truthE = -1, truthAngle = -1;

        if(cand.FS_index >= 0)
        {
          truthE = fs[cand.FS_index].energy.in<MeV>();
          truthAngle = fs[cand.FS_index].angle_wrt_z;

          pdg = fs[cand.FS_index].PDGCode;
          //Check for "GEANT neutron"s: FS particles that weren't neutrons but produced neutrons that I detected.
          if(pdg != 2112 && cand.dist_to_edep_as_neutron > 0_mm) pdg = std::numeric_limits<int>::max();
        }

        fFSPDG = pdg;
        fTruthE = truthE;
        fTruthAngle = truthAngle;

        const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
        fDistFromVertex = sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ)).in<mm>();
        fAngleWrtVertex = angle(cand, vertex);

        fEDep = cand.edep.in<MeV>();
        fDeltaT = cand.time.in<ns>();
        fNDigits = cand.nDigits;
        fHighestDigitE = cand.highestDigitE.in<MeV>();

        //Checked that angle() can return a negative number.  Consider deltaZ < 0.
        auto compareAngle = [this, &vertex](const auto& lhs, const auto& rhs)
                            {
                              return fabs(this->angle(lhs, vertex)) < fabs(this->angle(rhs, vertex));
                            };
        const auto smallestAnglePrefix = std::min_element(cands.begin(), whichCand, compareAngle);
        const auto smallestAnglePostfix = std::min_element(whichCand+1, cands.end(), compareAngle);
        fSmallestAngleDiff = 9999; //std::numeric_limits<double>::max();
        if(smallestAnglePrefix != cands.end()) fSmallestAngleDiff = fabs(angle(*whichCand, vertex) - angle(*smallestAnglePrefix, vertex));
        if(smallestAnglePostfix != cands.end()) fSmallestAngleDiff = std::min(fSmallestAngleDiff, fabs(angle(*whichCand, vertex) - angle(*smallestAnglePostfix, vertex)));

        fMCTree->Fill();
      }
    }
  }


  double PerCandidateTree::angle(const MCCandidate& cand, const units::LorentzVector<mm>& vertex) const
  {
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    //return deltaZ.in<mm>() / sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ)).in<mm>();
    return atan2(deltaZ.in<mm>(), cand.transverse.in<mm>());
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::PerCandidateTree> PerCandidateTree_reg("PerCandidateTree");
}
