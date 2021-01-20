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
#include "util/Interpolation.h"

//c++ includes
#include <fstream>

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
      GeneralizedBirksLaw(const YAML::Node& config, const typename CV::config_t& chw): CV(chw)
      {
        //Pre-load all PDG codes of interest here
        const std::vector<int> pdgsToLoad = {2212, 11, 1000020040};

        for(const int pdg: pdgsToLoad)
        {
          std::ifstream birksFile("birksRatios_" + std::to_string(pdg) + ".txt");
          if(birksFile) fPDGToBirksShift[pdg] = util::Interpolation(birksFile);
          else std::cerr << "GeneralizedBirksLaw: Failed to find a file of Birks' Law ratios for PDG code " << pdg << ".  Using a constant 1.\n";
        }
      }

      virtual ~GeneralizedBirksLaw() = default;

      std::vector<MeV> Getblob_edep() const override
      {
        struct Cand
        {
          MeV edep;
          int nCauses;
        };

        auto cands = Get<Cand>(CV::Getblob_edep(), CV::Getblob_n_causes());

        struct Cause
        {
          int pdgCode;
          MeV energy;
        };

        const auto causes = Get<Cause>(GetBlobCausePDGs(), GetBlobCauseEnergies());

        std::vector<MeV> shiftedEDeps;

        int nextCause = 0;
        for(auto& cand: cands)
        {
          MeV totalCauseEnergy = 0;

          //Energy-weighted sum of Birks' Law modified energies.
          double energyScaleFactor = 0.;
          for(int whichCause = nextCause; whichCause < nextCause + cand.nCauses; ++whichCause)
          {
            const auto& cause = causes[whichCause];
            energyScaleFactor += fPDGToBirksShift[cause.pdgCode][cause.energy.in<MeV>()] * cause.energy.in<MeV>();
            totalCauseEnergy += cause.energy;
          }
          //TODO: Should I weight by fraction of total edep instead of fraction of total cause energy?
          shiftedEDeps.push_back(cand.edep.in<MeV>() * energyScaleFactor / totalCauseEnergy.in<MeV>());

          nextCause += cand.nCauses;
        }

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
      mutable std::map<int, util::Interpolation> fPDGToBirksShift; //Has to be mutable so I can default to an empty Interpolation that just returns 1 for unhandled PDG codes.
  };
}

//If I share this systematic, this part doesn't go in the shared file.  Instead, it stays in my analysis package.
namespace
{
  plgn::Registrar<evt::WeightCachedUniverse, sys::GeneralizedBirksLaw, typename evt::WeightCachedUniverse::config_t> DropGEANTNeutrons_reg("GeneralizedBirksLaw");
}
