//File: RegularizeToCV.cpp
//Brief: Peels fit parameters out of the CV fit and pushes systematic universes towards the CV parameters
//Author: Andrew Olivier aolivier@ur.rochester.edu

//NucCCNeutrons includes
#include "fits/RegularizeToCV.h"
#include "util/units.h" //Kludge to fix problem with radians which comes up in vector.h
#include "util/mathWithUnits.h"

//ROOT includes
#include "Math/Minimizer.h"

//c++ includes
#include <cassert>

namespace fit
{
  RegularizeToCV::RegularizeToCV(const YAML::Node& config, const ROOT::Math::Minimizer& cvFit): RegularizationTerm(config),
                                                                                                fStrength(config["strength"].as<double>()),
                                                                                                fCVParams(cvFit.X(), cvFit.X() + cvFit.NDim()),
                                                                                                fCVErrors(cvFit.Errors(), cvFit.X() + cvFit.NDim())
  {
    assert(fCVParams.size() == fCVErrors.size() && "CV fitter must provide an error for each CV parameter.");
  }

  double RegularizeToCV::DoEval(const double* params) const
  {
    double term = 0;
    using namespace units;
    for(size_t whichParam = 0; whichParam < fCVParams.size(); ++whichParam) term += pow<2>(fCVParams[whichParam] - params[whichParam]) / pow<2>(fCVErrors[whichParam]);

    return term * fStrength;
  }
}
