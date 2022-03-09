//File: FailsQENeutronKinematics.cpp
//Brief: Remove events with neutron candidates that match the direction of missing momentum under a QE hypothesis.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cut includes
#include "cuts/reco/FailsQENeutronKinematics.h"

namespace
{
  using MeV2 = decltype(pow<2>(std::declval<MeV>()));
}

namespace reco
{
  FailsQENeutronKinematics::FailsQENeutronKinematics(const YAML::Node& config, const std::string& name): Cut(config, name),
                                                                                       fCandSelector(config["neutronCounter"]),
                                                                                       fBlobCosineDiffMin(config["blobCosineDiffMin"].as<double>())
  {
  }

  bool FailsQENeutronKinematics::checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const
  {
    const auto vertex = event.GetVtx();
    const auto pMu = event.GetMuonP();
    const MeV EnuQE = calcENuQE(event),
              pTMu = sqrt(pow<2>(pMu.x()) + pow<2>(pMu.y()));
    const double QECosine = 1./std::sqrt(1. + pow<2>(pTMu).in<MeV2>()/pow<2>(EnuQE - pMu.z()).in<MeV2>());

    //Look for the candidate with the cosine that is farthest from the predicted direction for a QE interaction.
    //See analyses/studies/QEAngleVersusNeutrons.cpp to learn why I chose this weird metric.
    const auto cands = event.Get<ana::NeutronMultiplicity::Candidate>(event.Getblob_edep(), event.Getblob_zPos(),
                                                                      event.Getblob_transverse_dist_from_vertex());

    double worstCosineDiff = 0;
    for(const auto& cand: cands)
    {
      if(fCandSelector.countAsReco(cand, vertex))
      {
        worstCosineDiff = std::max(fabs(CosineWrtZAxis(vertex, cand) - QECosine), worstCosineDiff);
      }
    }

    return worstCosineDiff < fBlobCosineDiffMin;
  }

  MeV FailsQENeutronKinematics::calcENuQE(const evt::Universe& univ) const
  {
    const MeV bindingE = 30_MeV, //MINERvA calculation for antineutrino only!  From Cheryl's paper.
              neutronMass = 939.6_MeV,
              protonMass = 938.3_MeV,
              muonMass = 105.66_MeV;

    return ((pow<2>(protonMass) - pow<2>(neutronMass - bindingE) - pow<2>(muonMass)).in<MeV2>() + 2.*((neutronMass - bindingE)*univ.GetMuonP().E()).in<MeV2>())
           /2./(neutronMass - bindingE - univ.GetMuonP().E() + univ.GetMuonP().z()).in<MeV>();
  }
}

namespace
{
  static reco::Cut::Registrar<reco::FailsQENeutronKinematics> FailsQENeutronKinematics_reg("FailsQENeutronKinematics");
}
