//File: Universe.h
//Brief: A single systematic Universe to be fit independently of all other Universes.
//       Combines Backgrounds and Sidebands into a chi squared function ROOT can fit.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_UNIVERSE_H
#define FIT_UNIVERSE_H

//fits includes
#include "fits/Sideband.h"

//ROOT includes
#include "Math/IFunction.h"

//c++ includes
#include <vector>

namespace ROOT
{
  namespace Math
  {
    class Minimizer;
  }
}

namespace fit
{
  class Background;

  //Based heavily on code by Aaron Bercellie at Ana/NukeCCPion/ana/ChainWrapper/src/SidebandFitter.cxx.  It's not on CVS though as of March 27, 2021.
  class Universe: public ROOT::Math::IBaseFunctionMultiDimTempl<double>
  {
    public:
      Universe(const std::vector<Sideband>& sidebands, std::vector<Background*> backgrounds, const double POTRatio,
               const int firstBin = 1, const int lastBin = -1);

      unsigned int NDim() const override;

      //Chi squared objective function that some ROOT fitter will optimize in parameters
      double DoEval(const double* parameters) const override;

      //Once the fit is complete, scale every histogram that was optimized based on the fit results
      void scale(Sideband& toModify, const ROOT::Math::Minimizer& fitParams) const;

      //Required for ROOT fittable function base class :(
      IBaseFunctionMultiDimTempl<double>* Clone() const override;

    private:
      std::vector<Sideband> fSidebands;
      std::vector<Background*> fBackgrounds; //Observer pointers

      double fPOTScale;
      int fFirstBin; //Bin to start sideband fit
      int fLastBin; //Last bin included in sideband fit
  };
}
#endif //FIT_UNIVERSE_H
