//File: Universe.h
//Brief: A Universe is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a MinervaUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef EVT_CVUNIVERSE_H
#define EVT_CVUNIVERSE_H

//TODO: Someone who maintains this code should deal with these warnings
#pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                            //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
                            #pragma GCC diagnostic ignored "-Woverloaded-virtual"
//PlotUtils includes
#include "PlotUtils/MinervaUniverse.h"
#include "PlotUtils/HistWrapper.h"
#pragma GCC diagnostic pop

//Get the unit definitions for my analysis
#include "util/units.h"

//c++ includes
#include <numeric>

namespace
{
  template <class UNIT>
  struct baseType
  {
    using result_t = typename UNIT::floating_point;
  };

  template <>
  struct baseType<int>
  {
    using result_t = int;
  };

  template <>
  struct baseType<double>
  {
    using result_t = double;
  };
}

//A quantity is just a label on top of a Plain Old Data type
//to help me keep track of units when programming.
//Convince PlotUtils::TreeWrapper to put POD types into matching
//quantities.
//I think this saves me some memory allocations down the line.
namespace PlotUtils
{
  namespace detail
  {
    template <class BASE_TAG, class PREFIX, class FLOATING_POINT>
    struct typeName<units::quantity<BASE_TAG, PREFIX, FLOATING_POINT>>
    {
      static const char* name; // = typeName<FLOATING_POINT>::name;
    };
  }
}

//Preprocessor macros so that I have only one point of maintenance for
//replacing ChainWrapper.
#define blobReco(BRANCH, TYPE)\
  virtual std::vector<TYPE> Get##BRANCH() const\
  {\
    auto branch = GetVec<TYPE>((blobAlg + "_" #BRANCH).c_str());\
    return dropCandidates(branch);\
  }

#define blobTruth(BRANCH, TYPE)\
  virtual std::vector<TYPE> Get##BRANCH() const\
  {\
    auto branch = GetVec<TYPE>(("truth_" + blobAlg + "_" #BRANCH).c_str());\
    return dropCandidates(branch);\
  }

#define truthMatched(BRANCH, TYPE)\
  virtual std::vector<TYPE> GetTruthMatched##BRANCH() const\
  {\
    return GetVec<TYPE>("truth_FS_" #BRANCH);\
  }

namespace evt
{
  struct SliceID;
  class Universe: public PlotUtils::MinervaUniverse
  {
    public:
      Universe(/*const std::string& blobAlg,*/ typename PlotUtils::MinervaUniverse::config_t chw, const double nsigma = 0);
      virtual ~Universe() = default;

      //Shared systematics components
      #include "PlotUtils/SystCalcs/WeightFunctions.h"
      #include "PlotUtils/SystCalcs/MuonFunctions.h"
      #include "PlotUtils/SystCalcs/TruthFunctions.h"

      //Configuration interfaces.  The design of the NSF prevents me from
      //doing all configuration in the constructor.
      inline static void SetBlobAlg(const std::string& newAlg) { blobAlg = newAlg; }
      inline void SetHypothesisName(const std::string& hypName) { fHypothesisName = hypName; }

      //MinervaUniverse interfaces
      //This is really used as "hypothesis name" for NeutrinoInt-based branches.
      virtual std::string GetAnaToolName() const override { return fHypothesisName; }
      virtual double GetRecoilEnergy() const; //override; //TODO: override again if I use the standard recoil systematics?

      //TODO: This hack seems to be necessary so that I can use the same universe, and thus the same HistWrapper<>, for multiple files.
      //The user is responsible for deleting m_chw as in its normal usage.
      void SetTree(PlotUtils::TreeWrapper* chw) { m_chw = chw; }

      //Information about this event
      SliceID GetEventID(const bool isData) const;

      //Hypothesis branches ported mostly from MECAnaTool
      //Reco branches
      virtual MeV GetRecoilE() const { return GetRecoilEnergy(); } //Put units on the NS Framework
      //TODO: Shift q0 in the same systematic universe that shifts GetRecoilEnergy().
      //q0 is different from GetRecoilE() because GetRecoilE() makes no attempt
      //to reconstruct the total energy of the outgoing hadronic system.  q0 is
      //therefore suitable for a calorimetric definition of neutrino energy while
      //GetRecoilE() is not.  Mechanically, this is done by the infamous
      //calorimetric spline which q0 uses and GetRecoilE() does not.
      virtual MeV Getq0() const { return GetDouble((GetAnaToolName() + "_q0Reco").c_str()); }
      virtual units::LorentzVector<mm> GetVtx() const { return units::LorentzVector<mm>(GetVec<double>("vtx")); }
      virtual ns GetMINOSTrackDeltaT() const { return GetDouble("minos_minerva_track_deltaT"); }
      virtual int GetNTracks() const { return GetInt("n_tracks"); }
      virtual int GetHelicity() const { return GetInt((GetAnaToolName() + "_nuHelicity").c_str()); }
      virtual units::LorentzVector<MeV> GetMuonP() const { return GetMuon4V(); }
      virtual radians GetMuonTheta() const { return GetThetamu(); }

      //Reco branches from CCQENu
      virtual bool hasInteractionVertex() const { return GetInt("has_interaction_vertex"); }
      virtual int GetNDeadDiscriminatorsUpstreamMuon() const { return GetInt("phys_n_dead_discr_pair_upstream_prim_track_proj"); }

      //More reco-only branches
      virtual MeV GetODEnergy() const { return GetDouble((GetAnaToolName() + "_OD_energy").c_str()); }
      virtual MeV GetIDECALEnergy() const { return GetDouble((GetAnaToolName() + "_Unused_ID_ECAL_energy").c_str()); }
      virtual MeV GetIDHCALEnergy() const { return GetDouble((GetAnaToolName() + "_Unused_ID_HCAL_energy").c_str()); }
      virtual MeV GetMuonFuzzEnergy() const { return GetDouble("muon_fuzz_energy"); }

      //Truth branches
      virtual MeV GetTruthQ3() const { return Getq3True(); }
      virtual MeV GetTruthQ0() const { return Getq0True(); }
      virtual typename units::detail::do_pow<2, GeV>::result_t GetTruthQ2() const { return GetQ2True(); }
      virtual GeV GetTruthEAvailable() const;
      virtual units::LorentzVector<mm> GetTruthVtx() const { return units::LorentzVector<mm>(GetVec<double>("mc_vtx")); }
      virtual int GetTruthTargetZ() const { return GetInt("mc_targetZ"); }
      virtual units::LorentzVector<MeV> GetTruthPmu() const;

      //Truth information from GENIE
      virtual int GetTruthNuPDG() const { return GetInt("mc_incoming"); }
      virtual int GetCurrent() const { return GetInt("mc_current"); }
      virtual int GetInteractionType() const { return GetInt("mc_intType"); }

      //Functions to retrieve per-candidate values in vector<>s.  Put them back together with get<>() in each Analysis.
      //Example: const auto cands = Get<NeutronCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_earliest_time());
      blobReco(blob_edep, MeV)
      blobReco(blob_calo_edep, MeV)
      blobReco(blob_transverse_dist_from_vertex, mm)
      blobReco(blob_first_muon_transverse, mm)
      blobReco(blob_zPos, mm)
      blobReco(blob_first_muon_long, mm)
      blobReco(blob_earliest_time, ns)
      blobReco(blob_nViews, int)
      blobReco(blob_n_clusters, int)
      blobReco(blob_n_digits, int)
      blobReco(blob_highest_digit_E, MeV)

      blobTruth(blob_geant_dist_to_edep_as_neutron, mm)
      blobTruth(blob_FS_index, int)
      blobTruth(blob_earliest_true_hit_time, ns)

      //Official FS particle branches.  These work in the Truth
      //tree as well as the "reco" tree.
      virtual std::vector<int> GetFSPDGCodes() const
      {
        return GetVec<int>("mc_FSPartPDG");
      }

      virtual std::vector<units::LorentzVector<MeV>> GetFSMomenta() const
      {
        return Get<units::LorentzVector<MeV>>(GetVec<double>("mc_FSPartPx"),
                                              GetVec<double>("mc_FSPartPy"),
                                              GetVec<double>("mc_FSPartPz"),
                                              GetVec<double>("mc_FSPartE"));
      }

      virtual std::vector<MeV> GetFSEnergies() const
      {
        const auto toConvert = GetVec<double>("mc_FSPartE");
        return std::vector<MeV>(toConvert.begin(), toConvert.end());
      }

      //Truth-matched branches for FS neutron energy loss study.
      //They only work in the "reco" tree.  Using them in the
      //Truth tree will give inconsistent results.
      truthMatched(PDG_code, int)
      truthMatched(energy, MeV)
      truthMatched(angle_wrt_z, double)
      truthMatched(edep, MeV)
      truthMatched(leaving_energy, GeV)
      truthMatched(late_energy, GeV)
      truthMatched(max_edep, MeV)
      truthMatched(elastic_loss, GeV)
      truthMatched(binding_energy, GeV)
      truthMatched(capture_energy, GeV)
      truthMatched(edep_before_birks, GeV)
      truthMatched(passive_loss, GeV)

      //Metadata using Universe constructor
      //Getting the flux integral in an MnvH1D is useful for the end of cross section extraction.
      //I want to get the integral for each flux universe individually and the CV integral for all
      //universes that do not vary the flux.  The flux integrals need to match the error bands and
      //binning of crossSectionHist.
      PlotUtils::MnvH1D* GetFluxIntegral(PlotUtils::HistWrapper<Universe>& crossSectionHist, const GeV Emin = 0_GeV, const GeV Emax = 100_GeV) const;

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
      //"Abandon all hope ye who enter here."
      template <class CAND, class TUPLE, size_t ...INDICES>
      std::vector<CAND> Get_impl(const TUPLE tuple, std::index_sequence<INDICES...> /*indices*/) const
      {
        const size_t nCands = std::get<0>(tuple).size();
        std::vector<CAND> result;

        for(size_t whichCand = 0; whichCand < nCands; ++whichCand) result.push_back(CAND{std::get<INDICES>(tuple)[whichCand]...});

        return result; 
      }

      //Drop neutron candidates to help me implement certain systematic universes.
      //Systematic universes have to modify fCandsToDrop rather than overload this
      //function because this needs to be a function template.
      template <class CAND>
      std::vector<CAND> dropCandidates(std::vector<CAND>& branch) const
      {
        auto toErase = branch.begin();
        for(size_t whichCand = 0; whichCand < branch.size(); ++whichCand)
        {
          if(fCandsToDrop.count(whichCand)) branch.erase(toErase);
          ++toErase;
        }

        return branch;
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
      std::string fHypothesisName;

      //Which neutron candidates to drop.  In the CV, no candidates are dropped.
      //Useful for systematic universes.
      std::set<int> fCandsToDrop;
  };
}

#endif //EVT_CVUNIVERSE_H
