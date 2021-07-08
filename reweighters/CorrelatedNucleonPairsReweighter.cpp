//File: CorrelatedNucleonPairsReweighter.h
//Brief: A Reweighter that changes the ratio of pp correlated nucleon pairs to np pairs.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef CORRELATEDNUCLEONPAIRSREWEIGHTER_H
#define CORRELATEDNUCLEONPAIRSREWEIGHTER_H

//PlotUtils includes
#include "PlotUtils/NSFDefaults.h"
#include "PlotUtils/MnvTuneSystematics.h"

//Reweighter includes
#include "PlotUtils/reweighters/Reweighter.h"

//util includes
#include "util/Factory.cpp"

//evt includes
#include "evt/Universe.h"

template <class UNIVERSE, class EVENT = PlotUtils::detail::empty>
class CorrelatedNucleonPairsReweighter: public PlotUtils::Reweighter<UNIVERSE, EVENT>
{
  public:
    CorrelatedNucleonPairsReweighter(const YAML::Node& config): PlotUtils::Reweighter<UNIVERSE, EVENT>(),
                                                                fOriginalNPFraction(config["OriginalNPFraction"].as<double>()),
                                                                fNewNPFraction(config["NewNPFraction"].as<double>())
    {
    }

    virtual ~CorrelatedNucleonPairsReweighter() = default;

    double GetWeight(const UNIVERSE& univ, const EVENT& /*event*/) const override
    {
      if(univ.GetInt("mc_intType")!=8) return 1.0;

      if(univ.GetInt("mc_targetZ")<2)
      {
        return 1.0; //There is no 2p2h on hydrogen.
      }

      bool isnnorpp = false;
      bool isnp = false;
      //now target analysis
      int target = univ.GetInt("mc_targetNucleon");
      if(target-2000000200==0 || target-2000000200==2) isnnorpp = true;
      if(target-2000000200==1) isnp = true;

      if(isnnorpp) return (1 - fNewNPFraction) / (1 - fOriginalNPFraction);
      else if(isnp) return fNewNPFraction / fOriginalNPFraction;

      //else that should never happen
      return 1.0;
    }

    std::string GetName() const override { return "CorrelatedNucleonPairs"; }
    bool DependsReco() const override { return false; }

  private:
    double fOriginalNPFraction;
    double fNewNPFraction;
};

namespace
{
  plgn::Registrar<PlotUtils::Reweighter<evt::Universe, PlotUtils::detail::empty>, CorrelatedNucleonPairsReweighter<evt::Universe, PlotUtils::detail::empty>> reg_nucleonPairReweight("CorrelatedNucleonPairsReweighter");
}

#endif //CORRELATEDNUCLEONPAIRSREWEIGHTER_H
