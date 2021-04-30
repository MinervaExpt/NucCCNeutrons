//File: RemoveQEByCandidates.cpp
//Brief: Remove event with neutron candidates that are likely to be from a neutral pion.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/RemoveQEByCandidates.h"

namespace reco
{
  RemoveQEByCandidates::RemoveQEByCandidates(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                                       fCandSelector(config["neutronCounter"]),
                                                                                       fQECosineMin(config["cosineMin"].as<double>()),
                                                                                       fQECosineMax(config["cosineMax"].as<double>()),
                                                                                       fQEDistanceMin(config["distanceMin"].as<mm>()),
                                                                                       fMaxQELikeCands(config["maxQECands"].as<int>())
  {
  }

  bool RemoveQEByCandidates::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const auto vertex = event.GetVtx();
    const auto cands = event.Get<NeutronCand>(event.Getblob_edep(), event.Getblob_zPos(),
                                              event.Getblob_transverse_dist_from_vertex());

    int nCandsInsideQECone = 0;
    for(const auto& cand: cands)
    {
      if(fCandSelector.countAsReco(cand, vertex))
      {
        const mm distFromVertex = sqrt(pow<2>(cand.transverse) + pow<2>(cand.z - vertex.z()));
        const double cosine = (cand.z - vertex.z()).in<mm>() / distFromVertex.in<mm>();
        if(cosine > fQECosineMin && cosine < fQECosineMax && distFromVertex > fQEDistanceMin) ++nCandsInsideQECone;
        //TODO: I could quit early here to save a little time
      }
    }

    return nCandsInsideQECone < fMaxQELikeCands;
  }
}

namespace
{
  static reco::Cut::Registrar<reco::RemoveQEByCandidates> RemoveQEByCandidates_reg("RemoveQEByCandidates");
}
