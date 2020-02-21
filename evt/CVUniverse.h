//File: CVUniverse.h
//Brief: A CVUniverse is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a DefaultCVUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef EVT_CVUNIVERSE_H
#define EVT_CVUNIVERSE_H

//PlotUtils includes
#include "PlotUtils/DefaultCVUniverse.h"

//Get the unit definitions for my analysis
#include "util/units.h"

//c++ includes
#include <numeric>

//Preprocessor macros so that I have only one point of maintenance for
//replacing ChainWrapper.
//TODO: I'd have to extend TreeWrapper to have a template Get<>()
//      for this to work with things like ROOT::Math::XYZTVector.
//      Probably not the most evil thing I've ever done.
#define blobReco(BRANCH, TYPE)\
  virtual std::vector<TYPE> Get##BRANCH() const\
  {\
    return GetVec<TYPE>((blobAlg + "_" #BRANCH).c_str());\
  }

#define blobTruth(BRANCH, TYPE)\
  virtual std::vector<TYPE> Get##BRANCH() const\
  {\
    return GetVec<TYPE>(("truth_" + blobAlg + "_" #BRANCH).c_str());\
  }

#define fs(BRANCH, TYPE)\
  virtual std::vector<TYPE> GetFS##BRANCH() const\
  {\
    return GetVec<TYPE>("truth_FS_" #BRANCH);\
  }

namespace evt
{
  struct weightCache
  {
    void SetEntry(PlotUtils::DefaultCVUniverse& cv)
    {
      //Make sure I don't call any universes' overrides by mistake
      minosEff = cv.DefaultCVUniverse::GetMinosEfficiencyWeight();
      GENIE = cv.DefaultCVUniverse::GetGenieWeight();
      fluxAndCV = cv.DefaultCVUniverse::GetFluxAndCVWeight();
      mec = cv.DefaultCVUniverse::Get2p2hWeight(); //Yes, I know 2p2h and MEC aren't exactly the same.  2p2h isn't a valid identifier though.
      rpa = cv.DefaultCVUniverse::GetRPAWeight();
    }

    double minosEff = 0;
    double GENIE = 0;
    double fluxAndCV = 0;
    double mec = 0;
    double rpa = 0;
  };

  class CVUniverse: public PlotUtils::DefaultCVUniverse
  {
    public:
      CVUniverse(/*const std::string& blobAlg,*/ PlotUtils::ChainWrapper* chw, const double nsigma = 0); //TODO: Get away from ChainWrapper?
      virtual ~CVUniverse() = default;

      //Configuration interfaces.  The design of the NSF prevents me from
      //doing all configuration in the constructor.
      inline static void SetBlobAlg(const std::string& newAlg) { blobAlg = newAlg; }

      //Too bad this can't be a constructor argument...
      void setWeightCache(weightCache& cache)
      {
        fWeightCache = &cache;
      }

      //DefaultCVUniverse interfaces
      //This is really used as "hypothesis name" for NeutrinoInt-based branches.
      virtual std::string GetAnaToolName() const override { return "CCNeutrons"; }
      virtual double GetRecoilEnergy() const override { return GetDouble("CCNeutrons_recoilE"); }

      //TODO: This hack seems to be necessary so that I can use the same universe, and thus the same HistWrapper<>, for multiple files.
      //The user is responsible for deleting m_chw as in its normal usage.
      void SetTree(PlotUtils::ChainWrapper* chw) { m_chw = chw; }

      //TODO: Some of these branches could be moved to a .cpp file.  Then, changing branch names would
      //      only force this file's unit to recompile.

      //Hypothesis branches ported mostly from MECAnaTool
      //TODO: Fix branches that come from derived values.  I need to calculate them from the most
      //      basic values I can find instead for the NS Framework.
      //Reco branches
      virtual MeV GetQ3() const { return GetDouble("CCNeutrons_q3"); } //TODO: I think this branch is derived from recoilE and Q^2
      virtual MeV GetRecoilE() const { return GetRecoilEnergy(); } //Put units on the NS Framework
      virtual vertex_t GetVtx() const { return vertex_t(GetVec<double>("vtx")); }
      virtual ns GetMINOSTrackDeltaT() const { return GetDouble("minos_minerva_track_deltaT"); }
      virtual int GetNTracks() const { return GetInt("n_tracks"); }
      virtual int GetHelicity() const { return GetInt("CCNeutrons_nuHelicity"); }
      virtual units::LorentzVector<MeV> GetMuonP() const { return GetMuon4V(); }

      //Truth branches
      virtual MeV GetTruthQ3() const { return Getq3True(); }
      virtual MeV GetTruthQ0() const { return Getq0True(); }
      virtual vertex_t GetTruthVtx() const { return vertex_t(GetVec<double>("mc_vtx")); }
      virtual int GetTruthTargetZ() const { return GetInt("mc_targetZ"); }
      virtual units::LorentzVector<GeV> GetTruthPmu() const { return momentum_t(GetVec<double>("mc_primFSLepton")); }
      virtual int GetTruthNuPDG() const { return GetInt("mc_incoming"); }

      //TODO: Deprecate this function with reweighter feature?
      virtual events GetWeight() const
      {
        //Rob told me that this is MnvGENIE v1.1 on 2/11/2020
        //TODO: I also need the non-resonant pion reweight for MnvGENIEv1
        return GetMinosEfficiencyWeight() * GetGenieWeight() * GetFluxAndCVWeight() * Get2p2hWeight() * GetRPAWeight();
      }

      //Override DefaultCVUniverse weight functions to used cached values as starting point.
      //This way, systematic universes only redo calculations they shift.
      virtual double GetMinosEfficiencyWeight() const override
      {
        return fWeightCache->minosEff;
      }

      virtual double GetGenieWeight() const override
      {
        return fWeightCache->GENIE;
      }

      #pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                            //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
      #pragma GCC diagnostic ignored "-Wunused-parameter"
      virtual double GetFluxAndCVWeight(double Enu = -99. /*GeV*/, int nu_pdg = -99) const override
      {
        return fWeightCache->fluxAndCV;
      }
      #pragma GCC diagnostic pop

      virtual double Get2p2hWeight() const override
      {
        return fWeightCache->mec;
      }

      virtual double GetRPAWeight() const override
      {
        return fWeightCache->rpa;
      }

      //TODO: Turn neutron candidates back on when I'm ready to try my multiplicity analysis
      //Functions to retrieve per-candidate values in vector<>s.  Put them back together with get<>() in each Analysis.
      //Example: const auto cands = Get<NeutronCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_earliest_time());
      blobReco(blob_edep, MeV)
      blobReco(blob_transverse_dist_from_vertex, mm)
      blobReco(blob_first_muon_transverse, mm)
      blobReco(blob_zPos, mm)
      blobReco(blob_first_muon_long, mm)
      blobReco(blob_earliest_time, ns)
      blobReco(blob_nViews, size_t)

      blobTruth(blob_geant_dist_to_edep_as_neutron, mm)
      blobTruth(blob_FS_index, int)
      blobTruth(blob_earliest_true_hit_time, ns)

      //Per FS particle branches
      fs(PDG_code, int)
      fs(angle_wrt_z, double)
      fs(edep, MeV)
      fs(energy, MeV)

      //Branches for FS neutron energy loss study
      fs(leaving_energy, GeV)
      fs(late_energy, GeV)
      fs(max_edep, MeV)
      fs(elastic_loss, GeV)
      fs(binding_energy, GeV)
      fs(capture_energy, GeV)
      fs(edep_before_birks, GeV)
      fs(passive_loss, GeV)

      //Unpack several related vector<> branches into an Analysis-specific struct called CAND.
      //One function to rule them all; one function to find them.
      //One function to bring them all and in the darkness bind them!

      //CAND is an Analysis-specific component that I want to retrieve several of.
      //It's basically a nice name for a std::tuple<>.  Think of a NeutronCandidate.
      //
      //FUNCTIONS... are any callable objects.  They're intended to be overridable
      //components that retrieve a vector<> with the value for each CAND.  Their types
      //must match the types of the numbers in CAND, and the compiler will enforce this.
      //
      //N.B.: I could use a "ValueProxy" with a template <class T> operator (T)() to rely on
      //      the user and whatever type checking I can do in FUNCTIONS to not even need
      //      return types from FUNCTIONS in principle.

    //Implementation details that not even my custom universes should ever see.
    private:
      weightCache* fWeightCache; //Central reference point for CV's weights

      //"Abandon all hope ye who enter here."
      template <class CAND, class TUPLE, size_t ...INDICES>
      std::vector<CAND> Get_impl(const TUPLE tuple, std::index_sequence<INDICES...> /*indices*/) const
      {
        const size_t nCands = std::get<0>(tuple).size();
        std::vector<CAND> result;

        for(size_t whichCand = 0; whichCand < nCands; ++whichCand) result.push_back(CAND{std::get<INDICES>(tuple)[whichCand]...});

        return result; 
      }

    public:
      template <class CAND, class ...FUNCTIONS>
      std::vector<CAND> Get(FUNCTIONS... branches) const
      {
        //Caching the entire vector from each branch at once probably avoids some overhead
        //no matter how you read from the TTree.  With ChainWrapper, it avoids a map lookup
        //for each CAND in exchange for making the compiler's job harder with std::make_index_sequence<>.
        auto cache = std::make_tuple(branches...);

        return Get_impl<CAND>(cache, std::make_index_sequence<sizeof...(FUNCTIONS)>{});
      }

    protected:
      //Name of the blob algorithm to use
      static std::string blobAlg;
  };
}

#endif //EVT_CVUNIVERSE_H
