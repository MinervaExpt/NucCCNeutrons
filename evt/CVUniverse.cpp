//File: CVUniverse.cpp
//Brief: A CVUniverse is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a DefaultCVUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Event model includes
#include "evt/CVUniverse.h"
#include "evt/EventID.h"

namespace evt
{
  std::string CVUniverse::blobAlg = "mergedTejinBlobs";

  CVUniverse::CVUniverse(/*const std::string& blobAlg,*/ PlotUtils::ChainWrapper* chw, const double nsigma): DefaultCVUniverse(chw, nsigma)
  {
  }

  //PlotUtils interface in MeV
  double CVUniverse::GetRecoilEnergy() const
  {
    const auto edeps = Getblob_calo_edep();
    return GetDouble("CCNeutrons_recoilE") - std::accumulate(edeps.begin(), edeps.end(), 0_MeV).in<MeV>();
  }

  GeV CVUniverse::GetTruthEAvailable() const
  {
    //Information I'm going to need about an FS particle
    struct FSPart
    {
      GeV energy;
      int pdgCode;
    };    

    const auto allFS = Get<FSPart>(GetFSenergy(), GetFSPDG_code());

    GeV E_avail = 0;
    for(const auto& fs: allFS)
    {
      if(abs(fs.pdgCode) == 211) E_avail += fs.energy - 139.57_MeV;
      else if(fs.pdgCode == 2212) E_avail += fs.energy - 938.3_MeV;
      else if(fs.pdgCode != 2112 && abs(fs.pdgCode) != 13) E_avail += fs.energy; //Don't count energy for an FS neutron.  It isn't availble for reconstruction.  Marvin doesn't seem to count the muon either.
    }

    return E_avail;
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
