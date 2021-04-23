//File: DropGENIENeutronCandidates.cpp
//Brief: The "GENIE modification" from figure 5 of https://arxiv.org/pdf/1901.04892.pdf.
//       Simulates MINERvA's neutrino event generator predicting fewer FS neutrons below
//       some kinetic energy threshold.  Watch out for the pseudo-random number generator!
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/Universe.h"

//util includes
#include "util/Factory.cpp"

//POSIX includes
#include <unistd.h>

//c++ includes
#include <random>
#include <chrono>

namespace sys
{
  class DropGENIENeutronCandidates: public evt::Universe
  {
    public:
      DropGENIENeutronCandidates(const YAML::Node& config, evt::Universe::config_t chw): evt::Universe(chw),
                                                                                            fMaxKEToDrop(config["MaxKEToDrop"].as<MeV>()),
                                                                                            fDist(config["ProbToDrop"].as<double>()),
                                                                                            fGen(std::chrono::system_clock::now().time_since_epoch().count() + getpid())
                                                                                            //TODO: Use PROCESS too so I don't get the same seed on Fermilab's grid nodes
      {
      }
      virtual ~DropGENIENeutronCandidates() = default;

      std::string ShortName() const override
      {
        return "DropGENIENeutronCandidates";
      }

      std::string LatexName() const override
      {
        return "Drop GENIE Neutrons";
      }

    private:
      void OnNewEntry() override
      {
        fCandsToDrop.clear();

        struct FSPart
        {
          GeV energy;
          int PDGCode;
        };

        const auto fs = Get<FSPart>(GetTruthMatchedenergy(), GetTruthMatchedPDG_code());
        const auto cands = Getblob_FS_index();

        for(size_t whichCand = 0; whichCand < cands.size(); ++whichCand)
        {
          const int fsIndex = cands[whichCand];
          if(fsIndex >= 0 && fs[fsIndex].PDGCode == 2112 && fs[fsIndex].energy - 939.6_MeV <= fMaxKEToDrop && fDist(fGen))
          {
            fCandsToDrop.insert(whichCand);
          }
        }
      }

      MeV fMaxKEToDrop;
      std::bernoulli_distribution fDist;
      std::mt19937 fGen;
  };
}

namespace
{
  plgn::Registrar<evt::Universe, sys::DropGENIENeutronCandidates, typename evt::Universe::config_t> DropGENIENeutronCandidates_reg("DropGENIENeutronCandidates");
}
