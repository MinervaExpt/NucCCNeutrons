//File: Piecewise.cpp
//Brief: A single scale factor fit across all bins of a background histogram during sideband fitting.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Fit includes
#include "fits/Piecewise.h"

//util includes
#include "util/mathWithUnits.h" //For pow<>()
#include "util/Factory.cpp"

//ROOT includes
#include "Math/Minimizer.h"

//c++ includes
#include <cassert>

namespace fit
{
  //TODO: Picewise doesn't work with 2 linear fits right now because of the way the background name is handled.  It's used in 2 places: getMostPureSideband() and guessInitialParameters().  It must match a histogram name in getMostPureSideband(), so it has to be the name of the background.  However, it must be unique in guessInitialParameters() because the fitter needs a parameter name.  I can't see a way to deal with this without making constructors more complicated across the board.  So, just don't use Piecewise with 2 ScaleFactors!
  //TODO: Turns out that Piecewise doesn't work with 2 LinearFits either because of parameter renaming.  That's going to be a problem.
  Piecewise::Piecewise(const YAML::Node& config, const std::string& name, const double sumBinWidths): Background(config, name, sumBinWidths), fBreakPoint(config["breakPoint"].as<double>()), fLowerFit(plgn::Factory<fit::Background, const std::string&, double>::instance().Get(config["lowerFit"], name, fBreakPoint)), fUpperFit(plgn::Factory<fit::Background, const std::string&, double>::instance().Get(config["upperFit"], name, sumBinWidths - fBreakPoint))
  {
  }

  int Piecewise::nPars() const
  {
    return fLowerFit->nPars() + fUpperFit->nPars();
  }

  double Piecewise::functionToFit(const double binCenter, const double* pars) const 
  {
    //This only works if fBreakpoint is on a bin edge.  If it were on a binCenter, then we'd have to calculate a derivative that probably doesn't exist.
    //TODO: This is hard to check for during setup.  Is it worth it?
    return (binCenter < fBreakPoint)?fLowerFit->functionToFit(binCenter, pars):fUpperFit->functionToFit(binCenter, pars + fLowerFit->nPars());
  }
                                                                                                                                                           
  double Piecewise::getSqErrOnFunction(const double binCenter, const double* pars, const double* parErrs) const 
  {
    using namespace units;
    return (binCenter < fBreakPoint)?fLowerFit->getSqErrOnFunction(binCenter, pars, parErrs):fUpperFit->getSqErrOnFunction(binCenter, pars + fLowerFit->nPars(), parErrs + fLowerFit->nPars());
  }
                                                                                                                                                           
  void Piecewise::guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const 
  {
    fLowerFit->guessInitialParameters(min, nextPar, sidebands, POTRatio);
    fUpperFit->guessInitialParameters(min, nextPar + fLowerFit->nPars(), sidebands, POTRatio);
  }
}

namespace
{
  fit::Background::Registrar<fit::Piecewise> reg_piecewise("Piecewise");
}
