//File: Background.h
//Brief: A Background scale factor calculation for sideband fits.  Derive from this class and implement functionToFit()
//       and other pure virtuals to use it.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_BACKGROUND_H
#define FIT_BACKGROUND_H

//util includes
#include "util/Factory.cpp"

//ROOT includes
#include "TH1D.h"

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//c++ includes
#include <memory>
#include <string>
#include <vector>

namespace ROOT
{
  namespace Math
  {
    class Minimizer;
  }
}

namespace fit
{
  class Sideband;

  class Background
  {
    public:
      //TODO: Get parameter limits from config
      Background(const YAML::Node& config, const std::string& bkgdName, const double /*sumBinWidths*/): name(bkgdName) {}
      virtual ~Background() = default;
  
      //functionToFit() tries to model a data/MC ratio for a specific background in a sideband.
      //It takes the bin number and nPars() parameters from the fitter as arguments.
      virtual double functionToFit(const double binCenter, const double* pars) const = 0;
      virtual double getSqErrOnFunction(const double binCenter, const double* pars, const double* parErrs) const = 0;
      virtual int nPars() const = 0;
  
      virtual void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const = 0;
  
      std::string name; //This background scales all histograms with this string in their names

      template <class DERIVED>
      using Registrar = plgn::Registrar<Background, DERIVED, const std::string&, double>;
  
    //Helper functions for guessing parameter starting values
    protected:
      std::unique_ptr<TH1D> makeDataMCRatio(const Sideband& sideband, const double POTRatio) const;
      auto getMostPureSideband(const std::vector<Sideband>& sidebands) const -> decltype(sidebands.begin());
  };
}

#endif //FIT_BACKGROUND_H
