//File: RegularizationTerm.h
//Brief: An extra term that's added on to the chi2 that the ROOT Minimizer optimizes for.
//       Regularization can enforce additional physical constraints on fits.  We also use
//       it for unfolding.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_REGULARIZATIONTERM_H
#define FIT_REGULARIZATIONTERM_H

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

namespace fit
{
  class RegularizationTerm
  {
    public:
      RegularizationTerm(const YAML::Node& /*config*/) {}
      virtual ~RegularizationTerm() = default;

      virtual double DoEval(const double* parameters) const = 0;
  };
}

#endif //FIT_REGULARIZATIONTERM_H
