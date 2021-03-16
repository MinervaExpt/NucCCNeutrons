//File: HasPi0Candidate.cpp
//Brief: Remove event with neutron candidates that are likely to be from a neutral pion.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/HasPi0Candidate.h"

namespace reco
{
  HasPi0Candidate::HasPi0Candidate(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                                       fCandSelector(config["neutronCounter"]),
                                                                                       fBlobAngleMin(config["blobAngleMin"].as<double>())
  {
  }

  bool HasPi0Candidate::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const auto vertex = event.GetVtx();
    const auto cands = event.Get<NeutronCand>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex(),
                                              event.Getblob_direction_difference());

    //int n3DCands = 0;
    for(const auto& cand: cands)
    {
      //Check only good neutron candidates with 3D fit information
      //TODO: Do I even need to apply neutron candidate selection when
      //      looking for pi0-induced blobs?
      if(cand.cosine_to_muon > -2 && fCandSelector.countAsReco(cand, vertex))
      {
        if(fabs(cand.cosine_to_muon) > fBlobAngleMin)
          return true;
        //++n3DCands;
      }
      //if(n3DCands > 1) return true;
    }

    return false;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::HasPi0Candidate> HasPi0Candidate_reg("HasPi0Candidate");
}
