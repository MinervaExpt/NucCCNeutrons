//File: FSDisappearingParticles.h
//Brief: A Study template that just produces the plots you need
//       to extract a differential cross section in 1D (TODO in an arbitrary
//       number of dimensions).  Use it to implement a FSDisappearingParticles
//       for your VARIABLE of interest.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "analyses/studies/FSDisappearingParticles.h"

namespace ana
{
  void FSDisappearingParticles::mcSignal(const evt::Universe& univ, const events weight)
  {
    const std::string prefix = "truth_hadronReweight";
    const auto finalEs = univ.GetVec<MeV>((prefix + "FinalE").c_str());
    const auto intCodes = univ.GetVecInt((prefix + "IntCodePerSegment").c_str()),
               trackIDs = univ.GetVecInt((prefix + "TrackID").c_str()),
               fsPDGs = univ.GetFSPDGCodes(),
               nSegmentsPerTraj = univ.GetVecInt((prefix + "NTrajPointsSaved").c_str());

    int lastSegment = -1;
    for(const int nSegments: nSegmentsPerTraj)
    {
      lastSegment += nSegments;
      if(lastSegment > 0 && intCodes[lastSegment] == 1) //Per HadronReweightTool, a particle whose trajectory ends with an inelastic scatter that doesn't produce the same PDG code with energy above stopping momentum
      {
        fStoppingEnergiesByPDG[fsPDGs[trackIDs[lastSegment]]].Fill(&univ, finalEs[lastSegment], weight);
      }
    }
  }

  void FSDisappearingParticles::afterAllFiles(const events /*passedSelection*/)
  {
    fStoppingEnergiesByPDG.visit([](auto& hist) { hist.SyncCVHistos(); });
  }

  void FSDisappearingParticles::mcBackground(const evt::Universe& univ, const background_t& background, const events weight)
  {
    const std::string prefix = "truth_hadronReweight";
    const auto finalEs = univ.GetVec<MeV>((prefix + "FinalE").c_str());
    const auto intCodes = univ.GetVecInt((prefix + "IntCodePerSegment").c_str()),
               trackIDs = univ.GetVecInt((prefix + "TrackID").c_str()),
               fsPDGs = univ.GetFSPDGCodes(),
               nSegmentsPerTraj = univ.GetVecInt((prefix + "NTrajPointsSaved").c_str());

    int lastSegment = -1;
    for(const int nSegments: nSegmentsPerTraj)
    {
      lastSegment += nSegments;
      if(lastSegment > 0)
      {
        fBackgroundStoppingEnergiesByFate[background][fsPDGs[trackIDs[lastSegment]]][intCodes[lastSegment]].Fill(&univ, finalEs[lastSegment], weight);
      }
    }
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::FSDisappearingParticles> FSDisappearingParticles_reg("FSDisappearingParticles");
}
