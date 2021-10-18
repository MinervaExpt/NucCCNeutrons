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
                                                                                util::NamedCategory<std::multiset<int>>{{{1000050110, 2112, 2212}}, "Bnp"},
                                                                                util::NamedCategory<std::multiset<int>>{{{1000040090, 1000020040}}, "BeAlpha"},
                                                                                util::NamedCategory<std::multiset<int>>{{{1000050120, 2212}}, "Bp"},
                                                                                util::NamedCategory<std::multiset<int>>{{{1000060110, 2112, 2112}}, "Cnn"}
                                                                               };

  std::vector<util::NamedCategory<int>> pdgToName = {util::NamedCategory<int>{{2212}, "Proton"},
                                                     util::NamedCategory<int>{{2112}, "Neutron"},
                                                     util::NamedCategory<int>{{211, -211}, "Charged Pion"},
                                                     util::NamedCategory<int>{{111}, "Neutral Pion"},
                                                    };

  constexpr MeV neutronMass = 939.6;
}

namespace ana
{
  MoNAReweightValidation::MoNAReweightValidation(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                   std::vector<background_t>& backgrounds, std::map<std::string,
                                   std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                         fTruthNeutronKEPerInteractionMode(childrenToChannelName, dir, "NeutronKEByChannel_TruthTree", "Neutron KE;Neutrons", 100, 1, 300, univs),
                                                                         fSelectedNeutronKEPerInteractionMode(childrenToChannelName, dir, "NeutronKEByChannel_SignalSelected", "Neutron KE;Neutrons", 100, 1, 300, univs),
                                                                         fFSParticleKEByPDGCode(pdgToName, dir, "FSParticleKE_Truth", "KE;Particles", 100, 1, 300, univs),
                                                                         fFiducial()
  {
    fTruthTotalNumberOfNeutrons = dir.make<Hist_t>("TruthTotalNumberOfNeutrons", "Truth Tree Neutron Inelastic Scatters;Neutron KE;Inelastic Scatters", 100, 1, 300, univs);
  }

  template <class FUNC>
  void MoNAReweightValidation::loopAllNeutrons(const evt::Universe& univ, const FUNC&& func)
  {
    const std::string prefix = "truth_neutronInelasticReweight"; //Beginning of branch names for inelastic reweighting
    const auto startEnergyPerPoint = univ.GetVec<MeV>((prefix + "InitialE").c_str());
    const auto xPerPoint = univ.GetVecDouble((prefix + "PosX").c_str()),
               yPerPoint = univ.GetVecDouble((prefix + "PosY").c_str()),
               zPerPoint = univ.GetVecDouble((prefix + "PosZ").c_str());


    const int nNeutrons = univ.GetInt((prefix + "NPaths").c_str());
    const auto nInelasticChildren = univ.GetVecInt((prefix + "NInelasticChildren").c_str()),
               allInelChildren = univ.GetVecInt((prefix + "InelasticChildPDGs").c_str()),
               nPointsPerNeutron = univ.GetVecInt((prefix + "NTrajPointsSaved").c_str()),
               materialPerPoint = univ.GetVecInt((prefix + "Nuke").c_str()),
               intCodePerPoint = univ.GetVecInt((prefix + "IntCodePerSegment").c_str());

    if(!nPointsPerNeutron.empty())
    {
      int endPoint = 0,
          endInelasticChild = 0;
      for(int whichNeutron = 0; whichNeutron < nNeutrons; ++whichNeutron)
      {
        const int startInelasticChild = endInelasticChild,
                  startPoint = endPoint;
        endInelasticChild += nInelasticChildren[whichNeutron];
        endPoint += nPointsPerNeutron[whichNeutron];

        //assert(endPoint > 0 && "Found a neutron with no trajectory points saved for reweighting!"); //Apparently, this really happens :(
        if(startPoint != endPoint)
        {
          const int intCode = intCodePerPoint[endPoint-1];

          //-6 is a special code for plastic scintillator inherited from MnvHadronReweight
          //int(eraction)Code checks that I'm only plotting inelastic interactions.  Elastic interactions should, and do, dominate over any individual inelastic channel.
          if(materialPerPoint[endPoint-1] == -6 && (intCode == 1 || intCode == 4) && fFiducial.InTracker(xPerPoint[endPoint-1], yPerPoint[endPoint-1], zPerPoint[endPoint-1]))
          {
            std::multiset<int> inelasticChildren(allInelChildren.begin() + startInelasticChild, allInelChildren.begin() + endInelasticChild);
            inelasticChildren.erase(22); //Ignore photons because GEANT tends to emit extra low energy photons to distribute binding energy

            //Break up very short-lived nuclei
            if(inelasticChildren.count(1000040080))
            {
              inelasticChildren.insert(1000020040);
              inelasticChildren.insert(1000020040);
              inelasticChildren.erase(1000040080);
            }

            func(univ, inelasticChildren, startEnergyPerPoint[endPoint - 1] - neutronMass);
          } //If in plastic scintillator
        } //If this neutron has at least 1 trajectory point
      } //For each neutron
    } //If there are some neutrons saved in this event
  } //loopAllNeutrons<>()

  void MoNAReweightValidation::mcSignal(const evt::Universe& univ, const events weight)
  {
    loopAllNeutrons(univ,
                    [this, weight](const evt::Universe& univ, const std::multiset<int>& inelasticChildren, const MeV startKE)
                    {
                      fSelectedNeutronKEPerInteractionMode[inelasticChildren].Fill(&univ, startKE, neutrons(weight.in<events>()));
                    });
  }

  void MoNAReweightValidation::truth(const evt::Universe& univ, const events weight)
  {
    loopAllNeutrons(univ,
                    [this, weight](const evt::Universe& univ, const std::multiset<int>& inelasticChildren, const MeV startKE)
                    {
                      fTruthNeutronKEPerInteractionMode[inelasticChildren].Fill(&univ, startKE, neutrons(weight.in<events>()));
                      fTruthTotalNumberOfNeutrons->Fill(&univ, startKE, neutrons(weight.in<events>()));
                    });

    const auto fs = univ.Get<FSPart>(univ.GetFSPDGCodes(), univ.GetFSMomenta());
    for(const auto& part: fs) fFSParticleKEByPDGCode[part.pdgCode].Fill(&univ, part.momentum.E() - part.momentum.mass(), neutrons(weight.in<events>())); //TODO: This is an abuse of my units system.  Create a "particles" unit?
  }

  void MoNAReweightValidation::afterAllFiles(const events /*passedSelection*/)
  {
    fTruthNeutronKEPerInteractionMode.visit([](auto& hist) { hist.SyncCVHistos(); });
    fSelectedNeutronKEPerInteractionMode.visit([](auto& hist) { hist.SyncCVHistos(); });
    fTruthTotalNumberOfNeutrons->SyncCVHistos();
    fFSParticleKEByPDGCode.visit([](auto& hist) { hist.SyncCVHistos(); });
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::MoNAReweightValidation> MoNAReweightValidation_reg("MoNAReweightValidation");
}
