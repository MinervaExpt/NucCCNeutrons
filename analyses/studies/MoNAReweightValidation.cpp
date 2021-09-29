//File: MoNAReweightValidation.h
//Brief: A Study to help understand the secondary particles that deposit the energy in neutron candidates.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/MoNAReweightValidation.h"

//util includes
#include "util/Factory.cpp"

//ROOT includes
#include "TDatabasePDG.h"

namespace
{
  /*std::vector<util::NamedCategory<int>> pdgToName = {util::NamedCategory<int>{{2212}, "Proton"},
                                                     util::NamedCategory<int>{{2112}, "Neutron"},
                                                     util::NamedCategory<int>{{22}, "Gamma"},
                                                     //util::NamedCategory<int>{{211, -211}, "Charged Pion"},
                                                     //util::NamedCategory<int>{{11, -11}, "Electron"},
                                                     util::NamedCategory<int>{{1000010020}, "Deuteron"},
                                                     util::NamedCategory<int>{{1000020040}, "Alpha"},
                                                     util::NamedCategory<int>{{1000060120}, "Carbon"},
                                                     util::NamedCategory<int>{{1000050100}, "Boron"},
                                                     util::NamedCategory<int>{{1000010030}, "Tritium"},
                                                     util::NamedCategory<int>{{1000040080, 1000040090, 1000040100}, "Beryllium"},
                                                    };*/

  /*std::map<std::string, std::multiset<int>> childrenToChannelName = {{"nGamma", {1000060120, 2112}},
                                                                     {"threeAlpha", {1000020040, 1000020040, 1000020040, 2112}},
                                                                     {"Bnp", {1000050110, 2112, 2212}}
                                                                    };*/

  std::vector<util::NamedCategory<std::multiset<int>>> childrenToChannelName = {util::NamedCategory<std::multiset<int>>{{{1000060120, 2112}}, "nGamma"},
                                                                                util::NamedCategory<std::multiset<int>>{{{1000020040, 1000020040, 1000020040, 2112}}, "threeAlpha"},
                                                                                util::NamedCategory<std::multiset<int>>{{{1000050110, 2112, 2212}}, "Bnp"}
                                                                               };

  constexpr MeV neutronMass = 939.6;
}

namespace ana
{
  MoNAReweightValidation::MoNAReweightValidation(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                   std::vector<background_t>& backgrounds, std::map<std::string,
                                   std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                         fTruthNeutronKEPerInteractionMode(childrenToChannelName, dir, "Neutron KE", "Among Signal Events", 20, 0, 4000, univs),
                                                                         fSelectedNeutronKEPerInteractionMode(childrenToChannelName, dir, "Neutron KE", "Among Selected Events", 20, 0, 4000, univs)
  {
  }

  void MoNAReweightValidation::mcSignal(const evt::Universe& univ, const events weight)
  {
    const std::string prefix = "truth_neutronInelasticReweight"; //Beginning of branch names for inelastic reweighting
    const auto startEnergyPerPoint = univ.GetVec<MeV>((prefix + "InitialE").c_str());

    const auto nInelasticChildren = univ.GetVecInt((prefix + "NInelasticChildren").c_str()),
               allInelChildren = univ.GetVecInt((prefix + "InelasticChildPDGs").c_str());

    int endInelasticChild = 0;
    for(int whichNeutron = 0; whichNeutron < startEnergyPerPoint.size(); ++whichNeutron)
    {
      const int startInelasticChild = endInelasticChild;
      endInelasticChild += nInelasticChildren[whichNeutron];

      std::multiset<int> inelasticChildren(allInelChildren.begin() + startInelasticChild, allInelChildren.begin() + endInelasticChild);
      inelasticChildren.erase(22); //Ignore photons because GEANT tends to emit extra low energy photons to distribute binding energy

      fSelectedNeutronKEPerInteractionMode[inelasticChildren].Fill(&univ, startEnergyPerPoint[whichNeutron] - neutronMass, weight);
    }
  }

  void MoNAReweightValidation::truth(const evt::Universe& univ, const events weight)
  {
    //TODO: Make this looping code into a function that takes a lamdba as an argument.  I may well have to upgrade it to be more specific.
    const std::string prefix = "truth_neutronInelasticReweight"; //Beginning of branch names for inelastic reweighting
    const auto startEnergyPerPoint = univ.GetVec<MeV>((prefix + "InitialE").c_str()); 

    const auto nInelasticChildren = univ.GetVecInt((prefix + "NInelasticChildren").c_str()),
               allInelChildren = univ.GetVecInt((prefix + "InelasticChildPDGs").c_str());

    int endInelasticChild = 0;
    for(int whichNeutron = 0; whichNeutron < startEnergyPerPoint.size(); ++whichNeutron)
    {
      const int startInelasticChild = endInelasticChild;
      endInelasticChild += nInelasticChildren[whichNeutron];

      std::multiset<int> inelasticChildren(allInelChildren.begin() + startInelasticChild, allInelChildren.begin() + endInelasticChild);
      inelasticChildren.erase(22); //Ignore photons because GEANT tends to emit extra low energy photons to distribute binding energy
      fTruthNeutronKEPerInteractionMode[inelasticChildren].Fill(&univ, startEnergyPerPoint[whichNeutron] - neutronMass, weight);
    }
  }

  void MoNAReweightValidation::afterAllFiles(const events /*passedSelection*/)
  {
    fTruthNeutronKEPerInteractionMode.visit([](auto& hist) { hist.SyncCVHistos(); });
    fSelectedNeutronKEPerInteractionMode.visit([](auto& hist) { hist.SyncCVHistos(); });
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::MoNAReweightValidation> MoNAReweightValidation_reg("MoNAReweightValidation");
}
