//File: ReweightGenerated2p2h.cpp
//Brief: A systematic universe that changes the ratio of interactions on nn/pp nucleon pairs to np pairs.
//       Originally designed to study whether more pp pairs in nature would change my cross section extraction results.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/Universe.h"

//util includes
#include "util/Factory.cpp"

namespace sys
{
  class ReweightGenerated2p2h: public evt::Universe
  {
    public:
      ReweightGenerated2p2h(const YAML::Node& config, evt::Universe::config_t chw): evt::Universe(chw),
                                                                                                fOriginalFractionNP(config["OriginalFractionNP"].as<double>(2./3.)),
                                                                                                fNewFractionNP(config["NewFractionNP"].as<double>())
      {
      }

      virtual ~ReweightGenerated2p2h() = default;

      //TODO: Override the GENIE weight instead?
      double GetLowRecoil2p2hWeight() const override
      {
        if(GetInt("mc_intType")!=8) return 1.0;
       
        if(GetInt("mc_targetZ")<2)
        {
          return 1.0; //There is no 2p2h on hydrogen.
        }
       
        bool isnnorpp = false;
        bool isnp = false;
        //now target analysis
        int target = GetInt("mc_targetNucleon");
        if(target-2000000200==0 || target-2000000200==2) isnnorpp = true;
        if(target-2000000200==1) isnp = true;
       
        if(isnnorpp) return (1 - fNewFractionNP) / (1 - fOriginalFractionNP);
        else if(isnp) return fNewFractionNP / fOriginalFractionNP;
        //else that should never happen
        return 1.0;
      }

      double GetWeightRatioToCV() const override
      {
        if(GetInt("mc_intType")!=8) return 1.0;

        if(GetInt("mc_targetZ")<2)
        {
          return 1.0; //There is no 2p2h on hydrogen.
        }

        bool isnnorpp = false;
        bool isnp = false;
        //now target analysis
        int target = GetInt("mc_targetNucleon");
        if(target-2000000200==0 || target-2000000200==2) isnnorpp = true;
        if(target-2000000200==1) isnp = true;

        //TODO: These might need to be divided by the CV weight?  I guess this isn't a systematic on any specific Reweighter though.
        //      It's currently designed to override the MINERvA 2p2h tune, but I don't think it should even do that.
        //TODO: Perhaps this is really a Reweighter rather than a Universe?  Seems like it.
        if(isnnorpp) return (1 - fNewFractionNP) / (1 - fOriginalFractionNP);
        else if(isnp) return fNewFractionNP / fOriginalFractionNP;

        //else that should never happen
        return 1.0;
      }

      std::string ShortName() const override
      {
        return "ReweightGenerated2p2h";
      }

      std::string LatexName() const override
      {
        return "Reweight Generated 2p2h";
      }

    private:
      double fOriginalFractionNP; //Fraction of generated 2p2h interactions that are on np pairs.
      double fNewFractionNP; //Reweight from original fraction to this fraction
  };
}

namespace
{
  plgn::Registrar<evt::Universe, sys::ReweightGenerated2p2h, typename evt::Universe::config_t> ReweightGenerated2p2h_reg("ReweightGenerated2p2h");
}
