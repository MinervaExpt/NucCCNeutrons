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

//Preprocessor macros so that I have only one point of maintenance for
//replacing ChainWrapper.
//TODO: I'd have to extend TreeWrapper to have a template Get<>()
//      for this to work with things like ROOT::Math::XYZTVector.
//      Probably not the most evil thing I've ever done.
#define blobReco(BRANCH, TYPE)\
  virtual std::vector<TYPE> Get##Branch() const\
  {\
    /*return Get<TYPE>(#BRANCH);*/\
    return GetVec<TYPE>(m_blobAlg + "_" #BRANCH);\
  }

#define blobTruth(BRANCH, TYPE)\
  virtual std::vector<TYPE> Get##BRANCH() const\
  {\
    return GetVec<TYPE>("truth_" + m_blobAlg + "_" #BRANCH);\
  }

#define fs(BRANCH, TYPE)\
  virtual std::vector<TYPE> GetFS##BRANCH() const\
  {\
    return GetVec<TYPE>("truth_FS_" #BRANCH);\
  }

namespace evt
{
  class CVUniverse: public PlotUtils::DefaultCVUniverse
  {
    public:
      CVUniverse(const std::string& blobAlg, PlotUtils::ChainWrapper* chw, const double nsigma = 0); //TODO: Get away from ChainWrapper?
      virtual ~CVUnvierse() = default;

      //TODO: Some of these branches could be moved to a .cpp file.  Then, changing branch names would
      //      only force this file's unit to recompile.

      //Hypothesis branches ported mostly from MECAnaTool
      //TODO: Fix branches that come from derived values.  I need to calculate them from the most
      //      basic values I can find instead for the NS Framework.
      //Reco branches
      virtual GeV GetQ3() const { return GetDouble("CCNeutrons_q3"); }
      virtual vertex_t GetVtx() const { return vertex_t(GetArray<double>("vtx")); }
      virtual ns GetMINOSTrackDeltaT() const { return GetDouble("minos_minerva_track_deltaT"); }
      virtual size_t GetNTracks() const { return GetInt("n_tracks"); }
      virtual int GetHelicity() const { return GetInt("CCNeutrons_nuHelicity"); }

      //Truth branches
      virtual GeV GetTruthQ3() const { return GetDouble("CCNeutrons_q3"); }
      virtual vertex_t GetTruthVtx() const { return vertex_t(GetArray<double>("mc_vtx")); }
      virtual int GetTruthTargetZ() const { return GetInt("mc_targetZ"); }
      virtual momentum_t GetTruthPmu() const { return momentum_t(GetArray<double>("mc_primFSLepton")); }

      virtual events GetWeight() const
      {
        //Taken from Ben's code at https://cdcvs.fnal.gov/redmine/projects/minerva-sw/repository/entry/Personal/bmesserl/SystematicsFramework/CVUniverse.h
        const MeV Enu = GetDouble("mc_incomingE");
        const int nu_type = GetInt("mc_incoming");
        const double fluxAndCV = GetFluxAndCVWeight(Enu, nu_type);
        return fluxAndCV * GetGenieWeight(); //TODO: Other weights?
      }

      //TODO: Turn neutron candidates back on when I'm ready to try my multiplicity analysis
      //Functions to retrieve per-candidate values in vector<>s.  Put them back together with get<>() in each Analysis.
      //Example: const auto cands = get<NeutronCandidate>(event.Getblob_edep, event.Getblob_zPos, event.Getblob_earliest_time);
      /*blobReco("blob_edep", MeV)
      blobReco("bob_transverse_dist_from_vertex", mm)
      blobReco("blob_first_muon_transverse", mm)
      blobReco("blob_zPos", mm)
      blobReco("blob_first_muon_long", mm)
      blobReco("blob_earliest_time", ns)
      blobReco("blob_nViews", size_t)

      blobTruth("blob_geant_dist_to_edep_as_neutron", mm)
      blobTruth("blob_FS_index", size_t)
      blobTruth("blob_earliest_true_hit_time", ns)

      //Per FS particle branches
      fs("PDG_Code", int)
      fs("angle_wrt_z", double)
      fs("edep", MeV)
      fs("energy", GeV)

      //Branches for FS neutron energy loss study
      fs("leaving_energy", GeV)
      fs("late_energy", GeV)
      fs("max_edep", MeV)
      fs("elastic_loss", GeV)
      fs("binding_energy", GeV)
      fs("capture_energy", GeV)
      fs("edep_before_birks", GeV)
      fs("passive_loss", GeV)

      //      What I would do without std::make_index_sequence<>: (thanks to https://stackoverflow.com/questions/49669958/details-of-stdmake-index-sequence-and-stdindex-sequence)
      //      template <size_t... which>
      //      struct index_sequence {};
      //
      //      template <size_t N_INDICES, size_t ...INDICES>
      //      struct index_helper
      //      {
      //        using type = index_helper<N_INDICES - 1ul, N_INDICES - 1ul, INDICES...>::type;
      //      };
      //
      //      template <size_t ...INDICES>
      //      struct index_helper<0ul, INDICES...>
      //      {
      //        using type = index_sequence<INDICES...>;
      //      };
      //
      //      template <size_t N_INDICES>
      //      using make_index_sequence<N_INDICES> = index_helper<N_INDICES>::type;
      //
      //      Seems like I'm all set!

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

      //TODO: If passing member function pointers isn't convenient, I can just pass the vectors themselves.
      template <class CAND, class ...FUNCTIONS>
      std::vector<CAND> Get(FUNCTIONS... branches)
      {
        //Caching the entire vector from each branch at once probably avoids some overhead
        //no matter how you read from the TTree.  With ChainWrapper, it avoids a map lookup
        //for each CAND in exchange for making the compiler's job harder with std::make_index_sequence<>.
        auto cache = std::make_tuple(FUNCTIONS()...);

        return get_impl(cache, std::make_index_sequence<sizeof...(FUNCTIONS)>{});
      }*/

    protected:
      //Name of the blob algorithm to use
      const std::string m_blobAlg;

    //Implementation details that not even my custom universes should ever see.
    private:
      //"Abandon all hope ye who enter here."
      template <class CAND, class TUPLE, class ...INDICES>
      std::vector<CAND> Get_impl(const TUPLE tuple, std::index_sequence<INDICES...> /*indices*/)
      {
        std::vector<CAND> result(std::get<0>(tuple).size());

        for(size_t whichCand = 0; whichCand < nCands; ++whichCand) result[whichCand] = {std::get<INDICES>(tuple)...};

        return result; 
      }
  };
}

#endif //EVT_CVUNIVERSE_H
