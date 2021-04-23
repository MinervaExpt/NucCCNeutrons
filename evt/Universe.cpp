//File: Universe.cpp
//Brief: A Universe is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a MinervaUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/PlotUtilsPhysicalConstants.h"

//Event model includes
#include "evt/Universe.h"
#include "evt/EventID.h"

//ROOT GenVector includes
#include "Math/AxisAngle.h"
#include "Math/Vector3D.h"

//Convince PlotUtils::TreeWrapper that a quantity can be read from a POD type.
//See Universe.h for a more detailed explanation.
namespace PlotUtils
{
  namespace detail
  {
    template <class BASE_TAG, class PREFIX, class FLOATING_POINT>
    const char* typeName<units::quantity<BASE_TAG, PREFIX, FLOATING_POINT>>::name = typeName<FLOATING_POINT>::name;
  }
}

namespace evt
{
  std::string Universe::blobAlg = "mergedTejinBlobs";

  Universe::Universe(/*const std::string& blobAlg,*/ typename MinervaUniverse::config_t chw, const double nsigma): MinervaUniverse(chw, nsigma)
  {
  }

  double Universe::GetRecoilEnergy() const
  {
    //return GetDouble((GetAnaToolName() + "_recoilE").c_str());
    return GetVecElem("recoil_summed_energy", 0); //CCQENu version
  }

  units::LorentzVector<MeV> Universe::GetTruthPmu() const
  {
    ROOT::Math::AxisAngle toBeamFrame(ROOT::Math::XYZVector(1., 0., 0.), MinervaUnits::numi_beam_angle_rad);
    units::LorentzVector<MeV> detectorFrame(GetVec<double>("mc_primFSLepton"));
    const auto beamFrame = toBeamFrame * detectorFrame.p().in<MeV>();
    return {beamFrame.x(), beamFrame.y(), beamFrame.z(), detectorFrame.E().in<MeV>()};
  }

  GeV Universe::GetTruthEAvailable() const
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
      //Implicitly exclude neutrons, nuclei, kaons, and heavy baryons
    }

    return std::max(0_GeV, E_avail);
  }

  SliceID Universe::GetEventID(const bool isData) const
  {
    SliceID id;
    id.run = GetInt(((isData?"ev":"mc") + std::string("_run")).c_str());
    id.subrun = GetInt(((isData?"ev":"mc") + std::string("_subrun")).c_str());
    id.gate = GetInt(isData?"ev_gate":"mc_nthEvtInFile");
    //TODO: Return a vector of SliceIDs instead?  A PhysicsEvent can be a combination of slices, but I've only ever seen 1 slice at a time in practice.
    id.slice = GetVecElemInt("slice_numbers", 0);

    return id;
  }

  MnvH1D* Universe::GetFluxIntegral(PlotUtils::HistWrapper<Universe>& crossSectionHist, const GeV Emin, const GeV Emax) const
  {
    bool useMuonCorrelations = true;

    if(GetAnalysisNuPDG() < 0)
    {
      std::cerr << "Muon momentum correlations are not yet ready for ME antineutrino analyses.  Turning them off for flux integral metadata.\n";
      useMuonCorrelations = false;
    }

    return PlotUtils::flux_reweighter(GetPlaylist(), GetAnalysisNuPDG(), UseNuEConstraint(), GetNFluxUniverses()).GetIntegratedFluxReweighted(GetAnalysisNuPDG(), crossSectionHist.hist, Emin.in<GeV>(), Emax.in<GeV>(), useMuonCorrelations);
  }
}
