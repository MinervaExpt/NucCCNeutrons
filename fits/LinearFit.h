//File: LinearFit.h
//Brief: A Background fit that fits a line to Sidebands' data/MC ratios.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_LINEARFIT_H
#define FIT_LINEARFIT_H

//fit includes
#include "fits/Background.h"

namespace fit
{
  class LinearFit: public Background
  {
    public:
      LinearFit(const YAML::Node& config, const std::string& name, const double sumBinWidths);
      virtual ~LinearFit() = default;

      double functionToFit(const double binCenter, const double* pars) const override;
      double getSqErrOnFunction(const double binCenter, const double* /*pars*/, const double* parErrs) const override;
      int nPars() const override { return 2; }
      void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const override;

    private:
      const double fSumBinWidths;

      //TODO: This delayed configuration is getting too verbose
      bool fHasFirstBinMin;
      double fFirstBinMin;
      bool fHasFirstBinMax;
      double fFirstBinMax;

      bool fHasLastBinMin;
      double fLastBinMin;
      bool fHasLastBinMax;
      double fLastBinMax;
  };
}

#endif //FIT_LINEARFIT_H
