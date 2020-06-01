//File: CVUniverse.cpp
//Brief: A CVUniverse is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a DefaultCVUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/PlotUtilsPhysicalConstants.h"

//Event model includes
#include "evt/CVUniverse.h"
#include "evt/EventID.h"

//ROOT GenVector includes
#include "Math/AxisAngle.h"
#include "Math/Vector3D.h"

namespace evt
{
  std::string CVUniverse::blobAlg = "mergedTejinBlobs";

  CVUniverse::CVUniverse(/*const std::string& blobAlg,*/ typename DefaultCVUniverse::config_t chw, const double nsigma): DefaultCVUniverse(chw, nsigma)
  {
  }

  double CVUniverse::GetRecoilEnergy() const
  {
    return GetDouble((GetAnaToolName() + "_recoilE").c_str());
    //return GetDouble((GetAnaToolName() + "_recoilE_withNukeCCCaloSpline").c_str());
    //return GetVecElem("recoil_summed_energy", 0);
  }

  MeV CVUniverse::GetEAvailable() const
  {
    const auto edeps = Getblob_edep();
    return std::max(0_MeV, GetRecoilE() - std::accumulate(edeps.begin(), edeps.end(), 0_MeV));
  }

  units::LorentzVector<MeV> CVUniverse::GetTruthPmu() const
  {
    ROOT::Math::AxisAngle toBeamFrame(ROOT::Math::XYZVector(1., 0., 0.), MinervaUnits::numi_beam_angle_rad);
    units::LorentzVector<MeV> detectorFrame(GetVec<double>("mc_primFSLepton"));
    const auto beamFrame = toBeamFrame * detectorFrame.p().in<MeV>();
    return {beamFrame.x(), beamFrame.y(), beamFrame.z(), detectorFrame.E().in<MeV>()};
  }

  GeV CVUniverse::GetTruthEAvailable() const
  {
    //Information I'm going to need about an FS particle
    struct FSPart
    {
      units::LorentzVector<MeV> momentum;
      int pdgCode;
    };    

    const auto allFS = Get<FSPart>(GetFSMomenta(), GetFSPDGCodes());

    GeV E_avail = 0;
    for(const auto& fs: allFS)
    {
      if(abs(fs.pdgCode) == 211) E_avail += fs.momentum.E() - 139.57_MeV;
      else if(fs.pdgCode == 2212) E_avail += fs.momentum.E() - 938.28_MeV;
      else if(fs.pdgCode == 111) E_avail += fs.momentum.E();
      else if(fs.pdgCode == 22) E_avail += fs.momentum.E();
      //Implicitly exclude nuclei, kaons, and heavy baryons
    }

    return std::max(0_GeV, E_avail);
  }

  SliceID CVUniverse::GetEventID(const bool isData) const
  {
    SliceID id;
    id.run = GetInt(((isData?"ev":"mc") + std::string("_run")).c_str());
    id.subrun = GetInt(((isData?"ev":"mc") + std::string("_subrun")).c_str());
    id.gate = GetInt(isData?"ev_gate":"mc_nthEvtInFile");
    //TODO: Return a vector of SliceIDs instead?  A PhysicsEvent can be a combination of slices, but I've only ever seen 1 slice at a time in practice.
    id.slice = GetVecElemInt("slice_numbers", 0);

    return id;
  }
}
