//File: QEAngleVersusNeutrons.cpp
//Brief: Can I get rid of some QE backgrounds by cutting out events with a neutron candidate that's aligned
//       with the missing energy from a hypothetical QE interaction?  This Study plots quantities I might
//       cut on to leverage QE kinematics as well as validates that the QE kinematics calculation itself
//       is correct.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "analyses/studies/QEAngleVersusNeutrons.h"
#include "analyses/studies/CandidateMath.h" //For CosineWrtZAxis()

namespace
{
  using MeV2 = decltype(pow<2>(std::declval<MeV>()));
}

namespace ana
{
  QEAngleVersusNeutrons::QEAngleVersusNeutrons(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                               std::vector<background_t>& backgrounds,
                                               std::map<std::string, std::vector<evt::Universe*>>& univs):
                                               Study(config, dir, std::move(mustPass), backgrounds, univs),
                                               fNeutronCounter(config["neutronCounter"]),
                                               fBackgroundNeutronAngleDiff(backgrounds, dir, "Background", "Reco Cosine Residual",
                                                                           config["cosineBinning"].as<std::vector<double>>(), univs),
                                               fBackgroundBestNeutronAngleDiff(backgrounds, dir, "BackgroundBestNeutron", "Reco Cosine Residual",
                                                                               config["cosineBinning"].as<std::vector<double>>(), univs)
  {
    fENuQEResolution = dir.make<units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, events>>("trueQEEnuResolution", "All True QE Events;E_{#nu};",
                                                                                                      config["EnuBinning"].as<std::vector<double>>(), univs);
    fMissingPCosineResolution = dir.make<PlotUtils::HistWrapper<evt::Universe>>("trueQEMissingPCosineResolution", "All True QE Events;Resolution on Missing Cosine;",
                                                                                                               config["cosineBinning"].as<std::vector<double>>(), univs);

    fSignalNeutronAngleDiff = dir.make<PlotUtils::HistWrapper<evt::Universe>>("signalNeutronAngleDiff", "Signal Candidate Residual;Reco Cosine Residual",
                                                                              config["cosineBinning"].as<std::vector<double>>(), univs);
    fSignalBestNeutronAngleDiff = dir.make<PlotUtils::HistWrapper<evt::Universe>>("signalBestNeutronAngleDiff", "Signal Best Candidate Residual;Reco Cosine Residual",
                                                                                  config["cosineBinning"].as<std::vector<double>>(), univs);
  }

  void QEAngleVersusNeutrons::mcSignal(const evt::Universe& event, const events weight)
  {
    const auto pMu = event.GetMuonP();
    const auto vertex = event.GetVtx();
    const MeV EnuQE = calcENuQE(event),
              pTMu = sqrt(pow<2>(pMu.x()) + pow<2>(pMu.y()));
    const double QECosine = 1./std::sqrt(1. + pow<2>(pTMu).in<MeV2>()/pow<2>(EnuQE - pMu.z()).in<MeV2>());

    if(event.GetInteractionType() == 1) //If event is true QE
    {
      units::LorentzVector<MeV> pNu(0., 0., event.GetEnuTrue(), event.GetEnuTrue());
      const auto trueMissingP = (pNu - event.GetTruthPmu()).p();
      fMissingPCosineResolution->FillUniverse(&event, fabs(cos(trueMissingP.theta()) - QECosine), weight.in<events>());
      fENuQEResolution->Fill(&event, fabs(MeV(event.GetEnuTrue()) - EnuQE), weight);
    }

    const auto cands = event.Get<ana::NeutronMultiplicity::Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
    bool setBestCosine = false;
    //mm bestDistFromVtx = 1e6;
    //MeV largestEDep = 0;
    //double bestCosineDiff = 1e6;
    double worstCosineDiff = 0;
    for(const auto& cand: cands)
    {
      if(fNeutronCounter.countAsReco(cand, vertex))
      {
        const auto neutronCosine = CosineWrtZAxis(vertex, cand);
        fSignalNeutronAngleDiff->FillUniverse(&event, fabs(neutronCosine - QECosine), weight.in<events>());

        //const mm dist2DFromVtx = DistFromVertex(vertex, cand);
        if(fabs(neutronCosine - QECosine) > worstCosineDiff) //fabs(neutronCosine - QECosine) < bestCosineDiff) //cand.edep > largestEDep) //dist2DFromVtx < bestDistFromVtx)
        {
          //bestDistFromVtx = dist2DFromVtx;
          //largestEDep = cand.edep;
          //bestCosineDiff = fabs(neutronCosine - QECosine);
          worstCosineDiff = fabs(neutronCosine - QECosine);
          setBestCosine = true;
        }
      }
    }

    if(setBestCosine) fSignalBestNeutronAngleDiff->FillUniverse(&event, worstCosineDiff, /*bestCosineDiff,*/ weight.in<events>());
  }

  void QEAngleVersusNeutrons::mcBackground(const evt::Universe& event, const background_t& background, const events weight)
  {
    const auto pMu = event.GetMuonP();
    const auto vertex = event.GetVtx();
    const MeV EnuQE = calcENuQE(event),
              pTMu = sqrt(pow<2>(pMu.x()) + pow<2>(pMu.y()));
    const double QECosine = 1./std::sqrt(1. + pow<2>(pTMu).in<MeV2>()/pow<2>(EnuQE - pMu.z()).in<MeV2>());
                                                                                                                                                                
    if(event.GetInteractionType() == 1) //If event is true QE
    {
      units::LorentzVector<MeV> pNu(0., 0., event.GetEnuTrue(), event.GetEnuTrue());
      const auto trueMissingP = (pNu - event.GetTruthPmu()).p();
      fMissingPCosineResolution->FillUniverse(&event, fabs(cos(trueMissingP.theta()) - QECosine), weight.in<events>());
      fENuQEResolution->Fill(&event, fabs(MeV(event.GetEnuTrue()) - EnuQE), weight);
    }
                                                                                                                                                                
    const auto cands = event.Get<ana::NeutronMultiplicity::Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
    bool setBestCosine = false;
    //mm bestDistFromVtx = 1e6;
    //MeV largestEDep = 0;
    //double bestCosineDiff = 1e6;
    double worstCosineDiff = 0;
    for(const auto& cand: cands)
    {
      if(fNeutronCounter.countAsReco(cand, vertex))
      {
        const auto neutronCosine = CosineWrtZAxis(vertex, cand);
        fBackgroundNeutronAngleDiff[background].FillUniverse(&event, fabs(neutronCosine - QECosine), weight.in<events>());

        //const mm dist2DFromVtx = DistFromVertex(vertex, cand);
        if(fabs(neutronCosine - QECosine) > worstCosineDiff) //fabs(neutronCosine - QECosine) < bestCosineDiff) //cand.edep > largestEDep) //dist2DFromVtx < bestDistFromVtx)
        {
          //bestDistFromVtx = dist2DFromVtx;
          //largestEDep = cand.edep;
          worstCosineDiff = fabs(neutronCosine - QECosine);
          //bestCosineDiff = fabs(neutronCosine - QECosine);
          setBestCosine = true;
        }
      }
    }

    if(setBestCosine) fBackgroundBestNeutronAngleDiff[background].FillUniverse(&event, worstCosineDiff, /*bestCosineDiff,*/ weight.in<events>());
  }

  void QEAngleVersusNeutrons::afterAllFiles(const events /*passedSelection*/)
  {
    fENuQEResolution->SyncCVHistos();
    fSignalNeutronAngleDiff->SyncCVHistos();
    fBackgroundNeutronAngleDiff.visit([](auto& hist) { hist.SyncCVHistos(); });
  }

  MeV QEAngleVersusNeutrons::calcENuQE(const evt::Universe& univ) const
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
  static ana::Study::Registrar<ana::QEAngleVersusNeutrons> QEAngleVersusNeutrons_reg("QEAngleVersusNeutrons");
}
