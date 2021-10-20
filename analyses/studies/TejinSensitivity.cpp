//File: TejinSensitivity.h
//Brief: A Study to help understand the secondary particles that deposit the energy in neutron candidates.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/TejinSensitivity.h"

//util includes
#include "util/Factory.cpp"

//ROOT includes
#include "TDatabasePDG.h"

namespace
{
  constexpr MeV neutronMass = 939.6_MeV;
}

namespace ana
{
  TejinSensitivity::TejinSensitivity(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                     std::vector<background_t>& backgrounds, std::map<std::string,
                                     std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                           fCuts(config["variable"])
  {
    fLeadingCandidateEDep = dir.make<Hist_t>("LeadingCandidateEDep", "Energy Deposit of Highest EDep Candidate", 65, 0, 2000, univs);
    fLeadingNeutronKE     = dir.make<Hist_t>("LeadingNeutronKE", "KE of Highest Energy FS Neutron", 65, 0, 2000, univs);
    fLeadingCandidateParentKE = dir.make<Hist_t>("LeadingCandidateParentKE", "KE of FS Neutron that Caused Highest EDep Candidate", 65, 0, 2000, univs);
  }

  void TejinSensitivity::mcSignal(const evt::Universe& event, const events weight)
  {
    auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                        event.Getblob_transverse_dist_from_vertex(),
                                        event.Getblob_FS_index(),
                                        event.Getblob_geant_dist_to_edep_as_neutron());
    auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetFSMomenta());
    const auto vertex = event.GetVtx();

    const auto lastAcceptedCand = std::remove_if(cands.begin(), cands.end(),
                                                 [this, &vertex](const auto& cand) { return fCuts.countAsReco(cand, vertex); });
    const auto leadingCand = std::max_element(cands.begin(), lastAcceptedCand,
                                              [](const auto& lhs, const auto& rhs)
                                              { 
                                                return lhs.edep < rhs.edep;
                                              });

    if(leadingCand != lastAcceptedCand && leadingCand->FS_index > 0 && fCuts.countAsTruth(fs[leadingCand->FS_index]))
      fLeadingCandidateParentKE->Fill(&event, fs[leadingCand->FS_index].energy - ::neutronMass, neutrons(weight.in<events>()));

    //N.B.: std::remove_if() sorts the fs vector.  It makes the FS_index of each candidate useless in doing so.
    //      So, finding the leading neutron must be done after analyzing the leading candidate's parent.
    const auto lastFSNeutron = std::remove_if(fs.begin(), fs.end(), [this](const auto& fs) { return fCuts.countAsTruth(fs); });
    const auto leadingNeutron = std::max_element(fs.begin(), lastFSNeutron,
                                                 [](const auto& lhs, const auto& rhs) { return lhs.energy < rhs.energy; });
    if(leadingNeutron != lastFSNeutron) fLeadingNeutronKE->Fill(&event, leadingNeutron->energy - ::neutronMass, neutrons(weight.in<events>()));
  }

  void TejinSensitivity::data(const evt::Universe& event, const events weight)
  {
    auto cands = event.Get<RecoCandidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                          event.Getblob_transverse_dist_from_vertex());
    const auto vertex = event.GetVtx();

    const auto lastAcceptedCand = std::remove_if(cands.begin(), cands.end(),
                                                 [this, &vertex](const auto& cand) { return fCuts.countAsReco(cand, vertex); });
    const auto leadingCand = std::max_element(cands.begin(), lastAcceptedCand,
                                              [](const auto& lhs, const auto& rhs)
                                              { 
                                                return lhs.edep < rhs.edep;
                                              });
    if(leadingCand != lastAcceptedCand) fLeadingCandidateEDep->Fill(&event, leadingCand->edep, neutrons(weight.in<events>()));
  }

  void TejinSensitivity::afterAllFiles(const events /*passedSelection*/)
  {
    fLeadingCandidateEDep->SyncCVHistos();
    fLeadingNeutronKE->SyncCVHistos();
    fLeadingCandidateParentKE->SyncCVHistos();
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::TejinSensitivity> TejinSensitivity_reg("TejinSensitivity");
}
