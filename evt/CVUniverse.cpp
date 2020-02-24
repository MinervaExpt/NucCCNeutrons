//File: CVUniverse.cpp
//Brief: A CVUniverse is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a DefaultCVUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Event model includes
#include "evt/CVUniverse.h"
#include "evt/EventID.h"

//TODO: Remove me
#include "evt/arachne.h"

namespace evt
{
  std::string CVUniverse::blobAlg = "mergedTejinBlobs";

  CVUniverse::CVUniverse(/*const std::string& blobAlg,*/ PlotUtils::ChainWrapper* chw, const double nsigma): DefaultCVUniverse(chw, nsigma)
  {
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

    //TODO: Remove me
    if(ShortName() == "cv" && E_avail < 0.05_GeV && GetRecoilE() > 0.2_GeV)
    {
      std::cout << "Got a very small truth E_avail of " << E_avail << " while recoil energy reco is " << GetRecoilE() << ".  I used these particles:\n";
      for(const auto& fs: allFS) std::cout << "A " << fs.pdgCode << " with energy = " << fs.energy << "\n";

      const auto edeps = Getblob_edep();
      std::cout << "Total neutron candidate energy deposit is " << std::accumulate(edeps.begin(), edeps.end(), 0_MeV) << "\n";
      std::cout << "See this event at " << util::arachne(GetEventID(false), false) << "\n";
    }

    return E_avail;
  }

  SliceID CVUniverse::GetEventID(const bool isData) const
  {
    SliceID id;
    id.run = GetInt(((isData?"ev":"mv") + std::string("_run")).c_str());
    id.subrun = GetInt(((isData?"ev":"mv") + std::string("_subrun")).c_str());
    id.gate = GetInt(isData?"ev_gate":"mc_nthEvtInFile");
    //TODO: Return a vector of SliceIDs instead?  A PhysicsEvent can be a combination of slices, but I've only ever seen 1 slice at a time in practice.
    id.slice = GetVecElemInt("slice_numbers", 0);

    return id;
  }
}
