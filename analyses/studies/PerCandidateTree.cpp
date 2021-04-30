//File: PerCandidateTree.cpp
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/studies/PerCandidateTree.h"

//util includes
#include "util/Factory.cpp"

//c++ includes
#include <cmath>
#include <functional> //std::bind

using namespace units;

namespace ana
{
  PerCandidateTree::PerCandidateTree(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                 fCuts(config["variable"]),
                                                                                                 fBackgroundTrees(backgrounds, dir, "BackgroundPerCandidateTree", "One entry for each truth-matched neutron candidate")
  {
    fMCTree = dir.make<TTree>("SignalPerCandidateTree", "One entry for each truth-matched neutron candidate");

    connectMCBranches(*fMCTree);
    connectRecoBranches(*fMCTree);

    using namespace std::placeholders; //for _1 which forwards the first argument passed to the bound function
    fBackgroundTrees.visit(std::bind(&PerCandidateTree::connectMCBranches, this, _1));
    fBackgroundTrees.visit(std::bind(&PerCandidateTree::connectRecoBranches, this, _1));
  }

  void PerCandidateTree::connectMCBranches(TTree& tree)
  {
    tree.Branch("FSPDG", &fFSPDG);
    tree.Branch("TruthE", &fTruthE);
    tree.Branch("TruthAngle", &fTruthAngle);
    tree.Branch("EventWeight", &fEventWeight);
  }

  void PerCandidateTree::connectRecoBranches(TTree& tree)
  {
    tree.Branch("EDep", &fEDep);
    tree.Branch("DistFromVertex", &fDistFromVertex);
    tree.Branch("DeltaT", &fDeltaT);
    tree.Branch("AngleWrtVertex", &fAngleWrtVertex);
    tree.Branch("NDigits", &fNDigits);
    tree.Branch("NClusters", &fNClusters);
    tree.Branch("NCandidates", &fNCandidates);
    tree.Branch("HighestDigitE", &fHighestDigitE);
    tree.Branch("SmallestAngleDiff", &fSmallestAngleDiff);

    tree.Branch("EventWeight", &fEventWeight);
  }

  void PerCandidateTree::mcSignal(const evt::Universe& event, const events weight)
  {
    fEventWeight = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(),
                                              event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron(),
                                              event.Getblob_n_digits(), event.Getblob_n_clusters(),
                                              event.Getblob_highest_digit_E());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z());
    const auto vertex = event.GetVtx();

    fNCandidates = fCuts.reco(event).in<neutrons>();

    for(auto whichCand = cands.begin(); whichCand != cands.end(); ++whichCand)
    {
      if(fCuts.countAsReco(*whichCand, vertex))
      {
        fillMCBranches(*whichCand, fs, vertex);
        fillRecoBranches(whichCand, vertex, cands);
        fMCTree->Fill();
      }
    }
  }

  void PerCandidateTree::mcBackground(const evt::Universe& event, const background_t& background, const events weight)
  {
    auto& treeToFill = fBackgroundTrees[background];
    fEventWeight = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(),
                                              event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron(),
                                              event.Getblob_n_digits(), event.Getblob_n_clusters(),
                                              event.Getblob_highest_digit_E());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z());
    const auto vertex = event.GetVtx();

    fNCandidates = fCuts.reco(event).in<neutrons>();

    for(auto whichCand = cands.begin(); whichCand != cands.end(); ++whichCand)
    {
      if(fCuts.countAsReco(*whichCand, vertex))
      {
        fillMCBranches(*whichCand, fs, vertex);
        fillRecoBranches(whichCand, vertex, cands);
        treeToFill.Fill();
      }
    }
  }

  void PerCandidateTree::fillMCBranches(const MCCandidate& cand, const std::vector<FSPart>& fs, const units::LorentzVector<mm> vertex)
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
  }

  void PerCandidateTree::fillRecoBranches(const std::vector<MCCandidate>::const_iterator whichCand, const units::LorentzVector<mm> vertex, const std::vector<MCCandidate>& allCands)
  {
    const auto& cand = *whichCand;
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    fDistFromVertex = sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ)).in<mm>();
    fAngleWrtVertex = angle(cand, vertex);

    fEDep = cand.edep.in<MeV>();
    fDeltaT = cand.time.in<ns>();
    fNDigits = cand.nDigits;
    fNClusters = cand.nClusters;
    fHighestDigitE = cand.highestDigitE.in<MeV>();

    //Checked that angle() can return a negative number.  Consider deltaZ < 0.
    auto compareAngle = [this, &vertex](const auto& lhs, const auto& rhs)
                        {
                          return fabs(this->angle(lhs, vertex)) < fabs(this->angle(rhs, vertex));
                        };
    const auto smallestAnglePrefix = std::min_element(allCands.begin(), whichCand, compareAngle);
    const auto smallestAnglePostfix = std::min_element(whichCand+1, allCands.end(), compareAngle);
    fSmallestAngleDiff = 9999; //std::numeric_limits<double>::max();
    if(smallestAnglePrefix != allCands.end()) fSmallestAngleDiff = fabs(angle(*whichCand, vertex) - angle(*smallestAnglePrefix, vertex));
    if(smallestAnglePostfix != allCands.end()) fSmallestAngleDiff = std::min(fSmallestAngleDiff, fabs(angle(*whichCand, vertex) - angle(*smallestAnglePostfix, vertex)));
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
