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

  double Universe::GetCalRecoilEnergy() const
  {
    return GetDouble((GetAnaToolName() + "_recoilE").c_str());
    //return GetVecElem("recoil_summed_energy", 0); //CCQENu version
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

    //Updated to match https://minerva-docdb.fnal.gov/cgi-bin/sso/RetrieveFile?docid=30875&filename=low_recoil_20220719.pdf&version=1
    //This is different from what people told me a few years ago, but I guess that's just too bad :(
    GeV E_avail = 0;
    const GeV protonMass = 938.27201_MeV;
    //const GeV neutronMass = 939.56536_MeV;
    const GeV pionMass = 139.5701_MeV;
    for(const auto& fs: allFS)
    {
      if(abs(fs.pdgCode) == 11 || abs(fs.pdgCode) == 13) {} //Ignore leptons
      else if(abs(fs.pdgCode) > 1e9) {} //Ignore nuclear fragments
      else if(fs.pdgCode == 2212) E_avail += fs.momentum.E() - protonMass; //Proton
      else if(fs.pdgCode == 2112) {} //Ignore neutrons
      else if(abs(fs.pdgCode) == 211) E_avail += fs.momentum.E() - pionMass; //Charged pion
      else if(fs.pdgCode == 111) E_avail += fs.momentum.E(); //Neutral pion
      else if(fs.pdgCode == 22) E_avail += fs.momentum.E(); //Photon
      else if(abs(fs.pdgCode) == 321) E_avail += fs.momentum.E(); //Charged kaon
      else if(fs.pdgCode > 3000) E_avail += fs.momentum.E() - protonMass; //Strange baryons
      else if(fs.pdgCode < -3000) E_avail += fs.momentum.E() + protonMass; //Strange anti-baryons
      else if(fs.pdgCode == -2212) E_avail += fs.momentum.E() + protonMass; //Per Abbey, assume anti-protons annihilate
      else if(fs.pdgCode == -2112) E_avail += fs.momentum.E() + protonMass; //Per Abbey, assume anti-neutrons annihilate.  Use the proton mass to be consistent with Abbey's assertion that they could annihilate on either a proton or a neutron.
      else E_avail += fs.momentum.E(); //Anything else gets its total energy.  Abbey notes that this includes strange mesons.
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
