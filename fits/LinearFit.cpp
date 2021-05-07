//File: LinearFit.cpp
//Brief: A Background fit that fits a line to Sidebands' data/MC ratios.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//fit includes
#include "fits/LinearFit.h"

//util includes
#include "util/mathWithUnits.h" //For pow<>()
#include "util/Factory.cpp"

//ROOT includes
#include "Math/Minimizer.h"

//c++ includes
#include <cassert>

namespace fit
{
  double LinearFit::functionToFit(const double binCenter, const double* pars) const 
  {
    const double slope = (pars[1] - pars[0]) / fSumBinWidths;
    return pars[0] + slope * binCenter;
  }

  double LinearFit::getSqErrOnFunction(const double binCenter, const double* /*pars*/, const double* parErrs) const 
  {
    //Assuming no correlation between slope and intercept parameters or any other Backgrounds
    using namespace units;
    return pow<2>(parErrs[1] * binCenter / fSumBinWidths) + pow<2>((binCenter/fSumBinWidths + 1) * parErrs[0]);
  }

  void LinearFit::guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const 
  {
    const auto largestSideband = getMostPureSideband(sidebands);
    assert(largestSideband != sidebands.end());

    auto mcRatio = makeDataMCRatio(*largestSideband, POTRatio);
    const double firstBinGuess = mcRatio->GetBinContent(1);
    const double lastBinGuess = mcRatio->GetMaximum();
    //const double slopeGuess = (mcRatio->GetMaximum() - interceptGuess) / (mcRatio->GetXaxis()->GetBinCenter(mcRatio->GetMaximumBin()) - mcRatio->GetXaxis()->GetBinCenter(1));

    min.SetVariable(nextPar, (name + " first bin").c_str(), firstBinGuess, firstBinGuess/20.);
    //min.SetVariableLimits(nextPar, std::min(0.3, firstBinGuess), std::max(2., firstBinGuess)); //1.5);
    min.SetVariable(nextPar + 1, (name + " last bin").c_str(), lastBinGuess, lastBinGuess/20.);
    //min.SetVariableLimits(nextPar + 1, std::min(0.3, lastBinGuess), std::max(2., lastBinGuess)); //1.5);
  }
}

namespace
{
  fit::Background::Registrar<fit::LinearFit> reg_linearFit("LinearFit");
}
