//File: NeutronDetection.cpp
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <cmath>

//signal includes
#include "analyses/signal/NeutronDetection.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  std::vector<util::NamedCategory<int>> pdgCategories = {util::NamedCategory<int>{{2112}, "GENIE Neutrons"}, 
                                                         util::NamedCategory<int>{{std::numeric_limits<int>::max()}, "GEANT Neutrons"},
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

namespace sig
{
  NeutronDetection::NeutronDetection(const YAML::Node& config, util::Directory& dir, std::vector<background_t>& backgrounds,
                                     std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Signal(config, dir, backgrounds, univs),
                                                                                                   fCuts(config["variable"]),
                                                                                                   fPDGToObservables(pdgCategories, dir, "", "Reco", univs,
                                                                                                                     config["binning"]["edep"].as<std::vector<double>>(),
                                                                                                                     config["binning"]["angle"].as<std::vector<double>>(),
                                                                                                                     config["binning"]["beta"].as<std::vector<double>>())
  {
    const auto edepBins = config["binning"]["edep"].as<std::vector<double>>();
    const auto angleBins = config["binning"]["angle"].as<std::vector<double>>();
    const auto betaBins = config["binning"]["beta"].as<std::vector<double>>();

    fEffNumerator = dir.make<CandidateObservables>("EfficiencyNumerator", "Reco", univs, edepBins, angleBins, betaBins);
    fEffDenominator = dir.make<CandidateObservables>("EfficiencyDenominator", "Truth", univs, edepBins, angleBins, betaBins);
  }

  void NeutronDetection::mcSignal(const std::vector<evt::CVUniverse*>& univs)
  {
    //Cache weight for each universe
    std::map<evt::CVUniverse*, neutrons> weights;
    for(const auto univ: univs) weights[univ] = neutrons(univ->GetWeight().in<events>());

    //Physics objects I'll need
    const auto& event = *univs.front();
    const auto cands = event.Get<MCCandidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex(), event.Getblob_earliest_time(), event.Getblob_FS_index());
    const auto fs = event.Get<FSPart>(event.GetFSPDG_code(), event.GetFSenergy(), event.GetFSangle_wrt_z());
    const auto vertex = event.GetVtx();

    std::set<int> FSWithCands; //FS neutrons with 1 or more reconstructed candidates

    for(const auto& cand: cands)
    {
      if(fCuts.countAsReco(cand, vertex))
      {
        int pdg = std::numeric_limits<int>::max();
        if(cand.FS_index >= 0)
        {
          pdg = fs[cand.FS_index].PDGCode;
          if(fCuts.countAsTruth(fs[cand.FS_index])) FSWithCands.insert(cand.FS_index);
        }

        fPDGToObservables[pdg].Fill(weights, cand, vertex);
      }
    }

    for(const auto& withCands: FSWithCands) fEffNumerator->Fill(weights, fs[withCands]);

    for(const auto& part: fs)
    {
      if(fCuts.countAsTruth(part)) fEffDenominator->Fill(weights, part);
    }
  }

  NeutronDetection::CandidateObservables::CandidateObservables(const std::string& name, const std::string& title, std::map<std::string, std::vector<evt::CVUniverse*>>& univs,
                                                               const std::vector<double>& edepBins, const std::vector<double>& angleBins,
                                                               const std::vector<double>& betaBins): fEDeps((name + "EDeps").c_str(), (title + " Energy Deposit").c_str(), edepBins, univs),
                                                                                                     fAngles((name + "Angles").c_str(), (title + " Angle w.r.t. Z Axis [radians]").c_str(), angleBins, univs),
                                                                                                     fBeta((name + "Beta").c_str(), (title + " Beta").c_str(), betaBins, univs)
  {
  }

  void NeutronDetection::CandidateObservables::Fill(std::map<evt::CVUniverse*, neutrons>& univs, const MCCandidate& cand, const units::LorentzVector<mm>& vertex)
  {
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    const double dist = std::sqrt(pow<2>(cand.transverse.in<mm>()) + pow<2>(deltaZ.in<mm>()));
    const double angle = deltaZ.in<mm>() / std::sqrt(pow<2>(cand.transverse.in<mm>()) + pow<2>(deltaZ.in<mm>()));
    const double beta = cand.time.in<ns>() / dist / 300.; //Speed of light is 300mm/ns

    for(const auto univ: univs)
    {
      fEDeps.Fill(univ.first, cand.edep, univ.second);
      fAngles.FillUniverse(univ.first, angle, univ.second.in<neutrons>());
      fBeta.FillUniverse(univ.first, beta, univ.second.in<neutrons>());
    }
  }

  void NeutronDetection::CandidateObservables::Fill(std::map<evt::CVUniverse*, neutrons>& univs, const FSPart& fs)
  {
    const double beta = std::sqrt(1. - pow<2>(939.6/fs.energy.in<MeV>())); //939.6 MeV is neutron mass

    for(const auto univ: univs)
    {
      fEDeps.Fill(univ.first, fs.energy, univ.second);
      fAngles.FillUniverse(univ.first, fs.angle_wrt_z, univ.second.in<neutrons>());
      fBeta.FillUniverse(univ.first, beta, univ.second.in<neutrons>());
    }
  }

  void NeutronDetection::CandidateObservables::SetDirectory(TDirectory* dir)
  {
    fEDeps.hist->SetDirectory(dir);
    fAngles.hist->SetDirectory(dir);
    fBeta.hist->SetDirectory(dir);
  }
}

//Register with Factory
namespace
{
  static plgn::Registrar<sig::Signal, sig::NeutronDetection, util::Directory&,
                         std::vector<typename sig::Signal::background_t>&,
                         std::map<std::string, std::vector<evt::CVUniverse*>>&> NeutronDetection_reg("NeutronDetection");
}
