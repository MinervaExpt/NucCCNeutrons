//File: DropGEANTNeutrons.cpp
//Brief: The "GEANT modification" from figure 5 of https://arxiv.org/pdf/1901.04892.pdf.
//       Simulates MINERvA's neutrino event generator predicting fewer FS neutrons below
//       some kinetic energy threshold.  Watch out for the pseudo-random number generator!
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/WeightCachedUniverse.h"

//util includes
#include "util/Factory.cpp"

//POSIX includes
#include <unistd.h>

//c++ includes
#include <random>
#include <chrono>

namespace sys
{
  class DropGEANTNeutrons: public evt::WeightCachedUniverse
  {
    public:
      DropGEANTNeutrons(const YAML::Node& config, evt::WeightCachedUniverse::config_t chw): evt::WeightCachedUniverse(chw),
                                                                                            fMaxEDepToDrop(config["MaxEDepToDrop"].as<MeV>()),
                                                                                            fDist(config["ProbToDrop"].as<double>()),
                                                                                            fGen(std::chrono::system_clock::now().time_since_epoch().count() + getpid())
                                                                                            //TODO: Use PROCESS too so I don't get the same seed on Fermilab's grid nodes
      {
      }
      virtual ~DropGEANTNeutrons() = default;

      std::string ShortName() const override
      {
        return "DropGEANTNeutrons";
      }

      std::string LatexName() const override
      {
        return "Drop GEANT Neutrons";
      }

    private:
      void OnNewEntry() override
      {
        fCandsToDrop.clear();

        struct Cand
        {
          MeV edep;
          mm distAsNeutron; //distAsNeutron < 0 => no neutrons in ancestry
        };

        const auto cands = Get<Cand>(Getblob_edep(), Getblob_geant_dist_to_edep_as_neutron());

        for(size_t whichCand = 0; whichCand < cands.size(); ++whichCand)
        {
          if(cands[whichCand].distAsNeutron > 0_mm && cands[whichCand].edep < fMaxEDepToDrop && fDist(fGen))
          {
            fCandsToDrop.insert(whichCand);
          }
        }
      }

      MeV fMaxEDepToDrop;
      std::bernoulli_distribution fDist;
      std::mt19937 fGen;
  };
}

namespace
{
  plgn::Registrar<evt::WeightCachedUniverse, sys::DropGEANTNeutrons, typename evt::WeightCachedUniverse::config_t> DropGEANTNeutrons_reg("DropGEANTNeutrons");
}
