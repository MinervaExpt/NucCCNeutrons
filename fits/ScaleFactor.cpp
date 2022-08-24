//File: ScaleFactor.cpp
//Brief: A single scale factor fit across all bins of a background histogram during sideband fitting.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Fit includes
#include "fits/ScaleFactor.h"

//util includes
#include "util/mathWithUnits.h" //For pow<>()
#include "util/Factory.cpp"

//ROOT includes
#include "Math/Minimizer.h"

//c++ includes
#include <cassert>

namespace fit
{
  ScaleFactor::ScaleFactor(const YAML::Node& config, const std::string& name, const double sumBinWidths): Background(config, name, sumBinWidths), fHasScaleMin(false), fHasScaleMax(false), fHasInitialGuess(false)
  {
    if(config.IsMap())
    {
      if(config["min"])
      {
        fHasScaleMin = true;
        fScaleMin = config["min"].as<double>();
      }
      if(config["max"])
      {
        fHasScaleMax = true;
        fScaleMax = config["max"].as<double>();
      }
      if(config["guess"])
      {
        fHasInitialGuess = true;
        fInitialGuess = config["guess"].as<double>();
      }
    }
  }

  double ScaleFactor::functionToFit(const double /*binCenter*/, const double* pars) const 
  {
    return pars[0];
  }
                                                                                                                                                           
  double ScaleFactor::getSqErrOnFunction(const double /*binCenter*/, const double* /*pars*/, const double* parErrs) const 
  {
    using namespace units;
    return pow<2>(parErrs[0]);
  }
                                                                                                                                                           
  void ScaleFactor::guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const 
  {
    const auto largestSideband = getMostPureSideband(sidebands);
    assert(largestSideband != sidebands.end());
                                                                                                                                                           
    auto mcRatio = makeDataMCRatio(*largestSideband, POTRatio);
    double scaleGuess = (mcRatio->GetMaximum() - mcRatio->GetMinimum())/2. + mcRatio->GetMinimum();
    if(fHasInitialGuess) scaleGuess = fInitialGuess; //User guess of parameter
    #ifndef NDEBUG
    std::cout << "Setting guess for scaled background " << name << " (index = " << nextPar << ") to " << scaleGuess << "\n"
              << "Ratio max is " << mcRatio->GetMaximum() << "\nRatio min is " << mcRatio->GetMinimum() << "\n";
    #endif //NDEBUG
                                                                                                                                                           
    if(fHasScaleMax) scaleGuess = std::min(fScaleMax, scaleGuess); //scaleGuess can't be larger than fScaleMax
    if(fHasScaleMin) scaleGuess = std::max(fScaleMin, scaleGuess); //scaleGuess can't be smaller than fScaleMin

    std::string varName = name + " scale factor";
    while(min.VariableIndex(varName) > -1) varName += " next fit";

    min.SetVariable(nextPar, varName, scaleGuess, scaleGuess/20.);
    if(fHasScaleMin)
    {
      if(fHasScaleMax) min.SetVariableLimits(nextPar, fScaleMin, fScaleMax);
      else min.SetVariableLowerLimit(nextPar, fScaleMin);
    }
    else if(fHasScaleMax) min.SetVariableUpperLimit(nextPar, fScaleMax);
  }
}

namespace
{
  fit::Background::Registrar<fit::ScaleFactor> reg_scaleFactor("ScaleFactor");
}
