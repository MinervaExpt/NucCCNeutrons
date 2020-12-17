//File: DropGENIENeutrons.cpp
//Brief: The "GENIE modification" from figure 5 of https://arxiv.org/pdf/1901.04892.pdf.
//       Simulates MINERvA's neutrino event generator predicting fewer FS neutrons below
//       some kinetic energy threshold.  Watch out for the pseudo-random number generator!
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/WeightCachedUniverse.h"

//util includes
#include "util/Factory.cpp"

namespace sys
{
  class DropGENIENeutrons: public evt::WeightCachedUniverse
  {
    public:
      DropGENIENeutrons(const YAML::Node& config, evt::WeightCachedUniverse::config_t chw): evt::WeightCachedUniverse(chw),
                                                                                            fMaxKEToDrop(config["MaxKEToDrop"].as<MeV>()),
                                                                                            fProbToDrop(config["ProbToDrop"].as<double>())
      {
      }
      virtual ~DropGENIENeutrons() = default;

      std::string ShortName() const override
      {
        return "DropGENIENeutrons";
      }

      std::string LatexName() const override
      {
        return "Drop GENIE Neutrons";
      }

    private:
      void OnNewEntry() override
      {
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
          //TODO: Also run a pseudo-random Bernoulli trial on the next line.
          if(fsIndex >= 0 && fs[fsIndex].PDGCode == 2112 && fs[fsIndex].energy - 939.6_MeV <= fMaxKEToDrop) fCandsToDrop.insert(whichCand);
        }
      }

      MeV fMaxKEToDrop;
      double fProbToDrop;
  };
}

namespace
{
  plgn::Registrar<evt::WeightCachedUniverse, sys::DropGENIENeutrons, typename evt::WeightCachedUniverse::config_t> DropGENIENeutrons_reg("DropGENIENeutrons");
}
