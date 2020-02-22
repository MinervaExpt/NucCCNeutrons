//File: CVUniverse.cpp
//Brief: A CVUniverse is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a DefaultCVUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Event model includes
#include "evt/CVUniverse.h"

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

    return E_avail;
  }
}
