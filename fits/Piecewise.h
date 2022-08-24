//File: Piecewise.h
//Brief: A single scale factor fit across all bins of a background histogram during sideband fitting.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_SCALEFACTOR_H
#define FIT_SCALEFACTOR_H

//Fit includes
#include "fits/Background.h"

namespace fit
{
  struct Piecewise: public Background
  {
    Piecewise(const YAML::Node& config, const std::string& name, const double sumBinWidths);
    virtual ~Piecewise() = default;
                                                                                                                                                             
    double functionToFit(const double /*binCenter*/, const double* pars) const override;
    double getSqErrOnFunction(const double /*binCenter*/, const double* /*pars*/, const double* parErrs) const override;
    int nPars() const override;
                                                                                                                                                             
    void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const override;

    private:
      double fBreakPoint; //Bin edge where break happens, and not bin number, because of how functionToFit() works
      std::unique_ptr<fit::Background> fLowerFit;
      std::unique_ptr<fit::Background> fUpperFit;
  };
}

#endif //FIT_SCALEFACTOR_H
