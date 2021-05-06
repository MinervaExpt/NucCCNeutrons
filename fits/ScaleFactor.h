//File: ScaleFactor.h
//Brief: A single scale factor fit across all bins of a background histogram during sideband fitting.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_SCALEFACTOR_H
#define FIT_SCALEFACTOR_H

//Fit includes
#include "fits/Background.h"

namespace fit
{
  struct ScaleFactor: public Background
  {
    ScaleFactor(const std::string& name, const double sumBinWidths): Background(name, sumBinWidths) {}
    virtual ~ScaleFactor() = default;
                                                                                                                                                             
    double functionToFit(const double /*binCenter*/, const double* pars) const override;
    double getSqErrOnFunction(const double /*binCenter*/, const double* /*pars*/, const double* parErrs) const override;
    int nPars() const override { return 1; }
                                                                                                                                                             
    void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const override;
  };
}

#endif //FIT_SCALEFACTOR_H
