//File: GeneralizedBirksLaw.cpp
//Brief: Birks' Law for photo-production quenching in scintillators lowers the energy deposits
//       we see for neutron candidates.  For very low energy particles, which have very high
//       stopping power, some authors like TODO
//       suggest an additional quadratic term in Birks' Law.  This can be modeled as a
//       particle-specific offset to energy deposit.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/WeightCachedUniverse.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  using CV = evt::WeightCachedUniverse; //In case I want to share this code with other analyses.
                                        //Make this a class template, name the parameter CV, derive from it,
                                        //and remove this block.
}

namespace sys
{
  class GeneralizedBirksLaw: public CV
  {
    public:
      GeneralizedBirksLaw(const YAML::Node& config, const typename CV::config_t& chw): CV(chw),
                                                                                          fEDepOffset(config["EDepOffset"].as<MeV>())
      {
      }

      virtual ~GeneralizedBirksLaw() = default;

      std::vector<MeV> Getblob_edep() const override
      {
        auto shiftedEDeps = CV::Getblob_edep();
        for(auto& edep: shiftedEDeps) edep -= fEDepOffset;
        return shiftedEDeps;
      }

      std::string ShortName() const override
      {
        return "GeneralizedBirksLaw";
      }

      std::string LatexName() const override
      {
        return "Generalized Birks' Law";
      }

    private:
      const MeV fEDepOffset; //For now, just subtract a constant energy deposit that
                             //I calculate externally.
      //TODO: Calculate fEDepToSubtract from C (see paper), particle type, and our Birks'
      //      Law constant from testbeam.  I may need a stopping power table for this.
      //      For alphas and protons, see ASTAR and PSTAR respectively.
      //      Doing this per particle type requires backtracking branches that I need
      //      to add to my AnaTuples.
  };
}

//If I share this systematic, this part doesn't go in the shared file.  Instead, it stays in my analysis package.
namespace
{
  plgn::Registrar<evt::WeightCachedUniverse, sys::GeneralizedBirksLaw, typename evt::WeightCachedUniverse::config_t> DropGEANTNeutrons_reg("GeneralizedBirksLaw");
}
