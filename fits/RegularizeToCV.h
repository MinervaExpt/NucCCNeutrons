//File: RegularizeToCV.h
//Brief: A RegularizationTerm that pushes each fit parameter towards the CV.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_REGULARIZETOCV_H
#define FIT_REGULARIZETOCV_H

#include "fits/RegularizationTerm.h"

namespace ROOT
{
  namespace Math
  {
    class Minimizer;
  }
}

namespace fit
{
  class RegularizeToCV: public RegularizationTerm
  {
    public:
      RegularizeToCV(const YAML::Node& config, const ROOT::Math::Minimizer& cvFit);
      virtual ~RegularizeToCV() = default;

      virtual double DoEval(const double* parameters) const override;

    private:
      double fStrength; //Adjusts how much the regularization affects the CV relative to the chi2 term
      std::vector<double> fCVParams;
      std::vector<double> fCVErrors;
  };
}

#endif //FIT_REGULARIZETOCV_H
