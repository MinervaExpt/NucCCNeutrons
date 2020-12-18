//File: NeutronDetection.cpp
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/studies/NeutronDetection.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  std::vector<util::NamedCategory<int>> pdgCategories = {util::NamedCategory<int>{{2112}, "Direct Neutrons"}, //"GENIE Neutrons"}, 
                                                         util::NamedCategory<int>{{std::numeric_limits<int>::max()}, "Reinteraction Neutrons"}, //"GEANT Neutrons"},
                                                         util::NamedCategory<int>{{-211, 211, 2212, 111,22}, "EM and Hadrons"},
                                                         util::NamedCategory<int>{{-13, 13}, "Muon"}
                                                        };

  template <int exponent>
  double pow(const double base)
  {
    static_assert(exponent > 0, "exponents < 0 not implemented yet");
    return base*pow<exponent - 1>(base);
  }

  //Base case to end recursion
  template <>
  double pow<0>(const double /*base*/)
  {
    return 1;
  }
}

namespace ana
{
  NeutronDetection::NeutronDetection(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fCuts(config["variable"]),
                                                                                                   fPDGToObservables(pdgCategories, dir, "", "Reco", univs,
                                                                                                                     config["binning"]["edep"].as<std::vector<double>>(),
                                                                                                                     config["binning"]["angle"].as<std::vector<double>>(),
                                                                                                                     config["binning"]["zDist"].as<std::vector<double>>(),
                                                                                                                     config["binning"]["beta"].as<std::vector<double>>())
  {
    const auto energyBins = config["binning"]["energy"].as<std::vector<double>>();
    const auto angleBins = config["binning"]["angle"].as<std::vector<double>>();
    const auto betaBins = config["binning"]["beta"].as<std::vector<double>>();

    fEffNumerator = dir.make<Efficiency>("EfficiencyNumerator", "Reco", univs, energyBins, angleBins, betaBins);
    fEffDenominator = dir.make<Efficiency>("EfficiencyDenominator", "Truth", univs, energyBins, angleBins, betaBins);

    fDataCands = dir.make<Observables>("Data", "Data;Reco", univs, config["binning"]["edep"].as<std::vector<double>>(),
                                       config["binning"]["angle"].as<std::vector<double>>(),
                                       config["binning"]["zDist"].as<std::vector<double>>(),
                                       config["binning"]["beta"].as<std::vector<double>>());

    fCandsPerFSNeutron = dir.make<units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, neutrons, events>>("CandsPerFSNeutron", "Candidates per FS Neutron;N Candidates;Events", 4, 0, 4, univs);
  }

  void NeutronDetection::data(const evt::Universe& event, const events weight)
  {
    const auto cands = event.Get<NeutronCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time());
    const auto vertex = event.GetVtx();

    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex)) fDataCands->Fill(event, weight.in<events>(), cand, vertex);
    }
  }

  void NeutronDetection::mcSignal(const evt::Universe& event, const events weight)
  {
    //Cache weight for each universe
    const neutrons weightPerNeutron = weight.in<events>();

    //Physics objects I'll need
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(), event.Getblob_FS_index(), event.Getblob_geant_dist_to_edep_as_neutron());
    const auto fs = event.Get<FSPart>(event.GetTruthMatchedPDG_code(), event.GetTruthMatchedenergy(), event.GetTruthMatchedangle_wrt_z());
    const auto vertex = event.GetVtx();

    std::set<int> FSWithCands; //FS neutrons with 1 or more reconstructed candidates
    std::multiset<int> candsPerFS; //One entry for each FS particle for each candidate it caused

    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex))
      {
        int pdg = -1; //-1 represents "Other".  Thsi happens when I couldn't find a parent PDG code.
                      //I think that means either overlay or cross-talk.
        if(cand.FS_index >= 0)
        {
          pdg = fs[cand.FS_index].PDGCode;
          //Check for "GEANT neutron"s: FS particles that weren't neutrons but produced neutrons that I detected.
          if(pdg != 2112 && cand.dist_to_edep_as_neutron > 0_mm) pdg = std::numeric_limits<int>::max();
          if(fCuts.countAsTruth(fs[cand.FS_index]))
          {
            FSWithCands.insert(cand.FS_index);
            candsPerFS.insert(cand.FS_index);
          }
        }

        fPDGToObservables[pdg].Fill(event, weightPerNeutron, cand, vertex);
      }
    }

    for(const auto& withCands: FSWithCands)
    {
      fCandsPerFSNeutron->Fill(&event, neutrons(candsPerFS.count(withCands)), weight);
      fEffNumerator->Fill(event, weightPerNeutron, fs[withCands]);
    }

    for(const auto& part: fs)
    {
      if(fCuts.countAsTruth(part)) fEffDenominator->Fill(event, weightPerNeutron, part);
    }
  }

  void NeutronDetection::afterAllFiles(const events passedSelection)
  {
    fPDGToObservables.visit([passedSelection](auto& hist)
                            {
                              hist.Scale(1./passedSelection.in<events>());
                              hist.SyncCVHistos();
                            });

    fDataCands->Scale(1./passedSelection.in<events>());
    fDataCands->SyncCVHistos();

    fEffNumerator->SyncCVHistos();
    fEffDenominator->SyncCVHistos();
  }

  NeutronDetection::Observables::Observables(const std::string& name, const std::string& title, std::map<std::string, std::vector<evt::Universe*>>& univs,
                                             const std::vector<double>& edepBins, const std::vector<double>& angleBins, const std::vector<double>& zBins,
                                             const std::vector<double>& betaBins): fEDeps((name + "EDeps").c_str(), (title + " Energy Deposit;candidates / event;").c_str(), edepBins, univs),
                                                                                   fAngles((name + "Angles").c_str(), (title + " Angle w.r.t. Z Axis [radians];candidates / event;").c_str(), angleBins, univs),
                                                                                   fBeta((name + "#beta").c_str(), (title + " #beta;candidates / event;").c_str(), betaBins, univs),
                                                                                   fZDistFromVertex((name + "ZDistFromVertex").c_str(), (title + " Z Distance;candidates / event;").c_str(), zBins, univs)
  {
  }

  /*void NeutronDetection::Observables::Fill(const evt::Universe& event, neutrons weightPerNeutron, const NeutronCandidate& cand, const units::LorentzVector<mm>& vertex)
  {
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    const double dist = std::sqrt(pow<2>(cand.transverse.in<mm>()) + pow<2>(deltaZ.in<mm>()));
    const double angle = deltaZ.in<mm>() / std::sqrt(pow<2>(cand.transverse.in<mm>()) + pow<2>(deltaZ.in<mm>()));
    const double beta = dist / cand.time.in<ns>() / 300.; //Speed of light is 300mm/ns

    fEDeps.Fill(&event, cand.edep, weightPerNeutron);
    fAngles.FillUniverse(&event, angle, weightPerNeutron.in<neutrons>());
    fBeta.FillUniverse(&event, beta, weightPerNeutron.in<neutrons>());
    fZDistFromVertex.Fill(&event, cand.z - vertex.z(), weightPerNeutron);
  }*/

  void NeutronDetection::Observables::SetDirectory(TDirectory* dir)
  {
    fEDeps.hist->SetDirectory(dir);
    fAngles.hist->SetDirectory(dir);
    fBeta.hist->SetDirectory(dir);
    fZDistFromVertex.hist->SetDirectory(dir);
  }

  void NeutronDetection::Observables::SyncCVHistos()
  {
    fEDeps.SyncCVHistos();
    fAngles.SyncCVHistos();
    fBeta.SyncCVHistos();
    fZDistFromVertex.SyncCVHistos();
  }

  void NeutronDetection::Observables::Scale(const double value, const char* option)
  {
    fEDeps.hist->Scale(value, option);
    fAngles.hist->Scale(value, option);
    fBeta.hist->Scale(value, option);
    fZDistFromVertex.hist->Scale(value, option);
  }


  NeutronDetection::Efficiency::Efficiency(const std::string& name, const std::string& title, std::map<std::string, std::vector<evt::Universe*>>& univs,
                                           const std::vector<double>& energyBins, const std::vector<double>& angleBins,
                                           const std::vector<double>& betaBins): fEnergies((name + "Energy").c_str(), (title + ";Kinetic Energy;FS Neutrons").c_str(), energyBins, univs),
                                                                                 fAngles((name + "Angles").c_str(), (title + ";Angle w.r.t. Z Axis [radians]; FS Neutrons").c_str(), angleBins, univs),
                                                                                 fBeta((name + "Beta").c_str(), (title + ";#beta;FS Neutrons").c_str(), betaBins, univs)
  {
  }

  void NeutronDetection::Efficiency::Fill(const evt::Universe& event, const neutrons weightPerNeutron, const FSPart& fs)
  {
    const auto neutronMass = 939.6_MeV;
    const double beta = std::sqrt(1. - pow<2>(neutronMass.in<MeV>()/fs.energy.in<MeV>())); //939.6 MeV is neutron mass

    fEnergies.Fill(&event, fs.energy - neutronMass, weightPerNeutron);
    fAngles.FillUniverse(&event, fs.angle_wrt_z, weightPerNeutron.in<neutrons>());
    fBeta.FillUniverse(&event, beta, weightPerNeutron.in<neutrons>());
  }

  void NeutronDetection::Efficiency::SetDirectory(TDirectory* dir)
  {
    fEnergies.hist->SetDirectory(dir);
    fAngles.hist->SetDirectory(dir);
    fBeta.hist->SetDirectory(dir);
  }

  void NeutronDetection::Efficiency::SyncCVHistos()
  {
    fEnergies.SyncCVHistos();
    fAngles.SyncCVHistos();
    fBeta.SyncCVHistos();
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::NeutronDetection> NeutronDetection_reg("NeutronDetection");
}
