//File: WeightCachedUniverse.h
//Brief: A Universe that defaults to using certain weights from the CV
//       rather than recalculating them.  Needs some extra support from
//       the event loop which ProcessAnaTuples provides.
//
//       More details: A weight can be cached only if it does not call
//                     any virtual functions.  At the moment, this means
//                     that weights from TruthFunctions.h are safe.
//                     The MINOS weight for example cannot be cached
//                     because it depends on a virtual function to get
//                     the reco MINOS muon momentum.  I used to cache it
//                     and broke a systematic this way!
//                     The event loop must call SetEntry() on the weight
//                     cache using the CV at the beginning of each event.
//
//       This class is defined in a header because I only ever expect to
//       add new functions.  When I add a new function, I have to recompile
//       everything that includes it anyway.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef EVT_WEIGHTCACHEDUNVIERSE_H
#define EVT_WEIGHTCACHEDUNVIERSE_H

//evt includes
#include "evt/Universe.h"

namespace evt
{
  struct WeightCache
  {
    void SetEntry(Universe& cv)
    {
      //Make sure I don't call any universes' overrides by mistake
      GENIE = cv.Universe::GetGenieWeight();
      fluxAndCV = cv.Universe::GetFluxAndCVWeight();
      mec = cv.Universe::GetLowRecoil2p2hWeight(); //Yes, I know 2p2h and MEC aren't exactly the same.  2p2h isn't a valid identifier though.
      rpa = cv.Universe::GetRPAWeight();
    }

    double GENIE = 0;
    double fluxAndCV = 0;
    double mec = 0;
    double rpa = 0;
  };

  class WeightCachedUniverse: public Universe
  {
    public:
      WeightCachedUniverse(/*const std::string& blobAlg,*/ typename PlotUtils::MinervaUniverse::config_t chw, const double nsigma = 0): Universe(chw, nsigma)
      {
      }

      //Too bad this can't be a constructor argument...
      //TODO: Maybe it can be a constructor argument now?
      void setWeightCache(WeightCache& cache)
      {
        fWeightCache = &cache;
      }

      //Each time I decide to cache a weight, I need to both add a function here and
      //add a place for it to be cached in WeightCached at the bottom of this file.
      virtual double GetGenieWeight() const override
      {
        return fWeightCache->GENIE;
      }

      #pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                                  //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
      #pragma GCC diagnostic ignored "-Wunused-parameter"
      virtual double GetFluxAndCVWeight(double Enu = -99., int nu_pdg = -99) const override
      {
        return fWeightCache->fluxAndCV;
      }
      #pragma GCC diagnostic pop

      virtual double GetLowRecoil2p2hWeight() const override
      {
        return fWeightCache->mec;
      }

      virtual double GetRPAWeight() const override
      {
        return fWeightCache->rpa;
      }

    private:
      WeightCache* fWeightCache; //Central reference point for CV's weights
  };
}

#endif //EVT_WEIGHTCACHEDUNVIERSE_H
