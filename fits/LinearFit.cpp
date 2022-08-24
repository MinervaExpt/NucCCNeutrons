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
  LinearFit::LinearFit(const YAML::Node& config, const std::string& name, const double sumBinWidths): Background(config, name, sumBinWidths), fSumBinWidths(sumBinWidths), fHasFirstBinMin(false), fHasFirstBinMax(false), fHasLastBinMin(false), fHasLastBinMax(false)
 {
   if(config.IsMap())
   {
     if(config["firstBin"])
     {
       if(config["firstBin"]["min"])
       {
         fHasFirstBinMin = true;
         fFirstBinMin = config["firstBin"]["min"].as<double>();
       }

       if(config["firstBin"]["max"])
       {
         fHasFirstBinMax = true;
         fFirstBinMax = config["firstBin"]["max"].as<double>();
       }
     }

     if(config["lastBin"])
     {
       if(config["lastBin"]["min"])
       {
         fHasLastBinMin = true;
         fLastBinMin = config["lastBin"]["min"].as<double>();
       }

       if(config["lastBin"]["max"])
       {
         fHasLastBinMax = true;
         fLastBinMax = config["lastBin"]["max"].as<double>();
       }
     }
   }
 }

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

    std::string firstVarName = name + " first bin";
    while(min.VariableIndex(firstVarName) > -1) firstVarName += " next fit";  //firstVarName might already be in use if this is used in a Piecewise Background
    min.SetVariable(nextPar, firstVarName, firstBinGuess, firstBinGuess/20.);

    if(fHasFirstBinMin)
    {
      if(fHasFirstBinMax) min.SetVariableLimits(nextPar, std::min(firstBinGuess, fFirstBinMin), std::max(firstBinGuess, fFirstBinMax));
      else min.SetVariableLowerLimit(nextPar, std::min(firstBinGuess, fFirstBinMin));
    }
    else if(fHasFirstBinMax) min.SetVariableUpperLimit(nextPar, std::max(firstBinGuess, fFirstBinMax));

    std::string lastVarName = name + " last bin";
    while(min.VariableIndex(lastVarName) > -1) lastVarName += " next fit"; //lastVarName might already be in use if this is used in a Piecewise Background
    min.SetVariable(nextPar + 1, lastVarName, lastBinGuess, lastBinGuess/20.);

    if(fHasLastBinMin)
    {
      if(fHasLastBinMax) min.SetVariableLimits(nextPar + 1, std::min(firstBinGuess, fLastBinMin), std::max(firstBinGuess, fLastBinMax));
      else min.SetVariableLowerLimit(nextPar + 1, std::min(firstBinGuess, fLastBinMin));  
    }
    else if(fHasLastBinMax) min.SetVariableUpperLimit(nextPar + 1, std::max(firstBinGuess, fLastBinMax));
  }
}

namespace
{
  fit::Background::Registrar<fit::LinearFit> reg_linearFit("LinearFit");
}
