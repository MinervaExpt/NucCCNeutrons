//File: Universe.h
//Brief: A single systematic Universe to be fit independently of all other Universes.
//       Combines Backgrounds and Sidebands into a chi squared function ROOT can fit.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//fits includes
#include "fits/Universe.h"
#include "fits/Background.h"
#include "fits/RegularizationTerm.h"

//util includes
#include "util/units.h" //Kludge to fix problem with radians which comes up in vector.h
#include "util/mathWithUnits.h"

//ROOT includes
#include "Math/Minimizer.h"

//c++ includes
#include <cassert>
#include <numeric> //std::accumulate in c++14
#include <algorithm> //std::accumulate in c++11

namespace fit
{
  Universe::Universe(const std::vector<Sideband>& sidebands, std::vector<Background*> backgrounds, const double POTRatio,
                     RegularizationTerm* regTerm, const int firstBin, const int lastBin): IBaseFunctionMultiDimTempl<double>(), fSidebands(sidebands), fBackgrounds(backgrounds), fRegTerm(regTerm), fPOTScale(POTRatio), fFirstBin(firstBin), fLastBin(lastBin)
  {
    //TODO: assert() that all sidebands have same binning
    assert(!fSidebands.empty() && "Requested sideband fit with no sidebands!");
    assert(std::all_of(fSidebands.begin(), fSidebands.end(), [&backgrounds](const auto& sideband) { return sideband.floatingHists.size() == backgrounds.size(); }));
    #ifndef NDEBUG
    for(size_t whichBkg = 0; whichBkg < fBackgrounds.size(); ++whichBkg)
    {
      for(const auto& sideband: fSidebands) assert(std::string(sideband.floatingHists[whichBkg]->GetName()).find(fBackgrounds[whichBkg]->name) != std::string::npos && "Sideband and background indices don't match!");
    }
    #endif //NDEBUG
    if(fLastBin < fFirstBin) fLastBin = fSidebands.front().data.GetXaxis()->GetNbins();
  }
 
  unsigned int Universe::NDim() const 
  {
    return std::accumulate(fBackgrounds.begin(), fBackgrounds.end(), 0l, [](const int sum, const Background* bkg) { return sum + bkg->nPars(); });
  }
 
  //Chi squared objective function that some ROOT fitter will optimize in parameters
  double Universe::DoEval(const double* parameters) const 
  {
    //Add the chi2 statistic for each sideband together with the others.  Assuming the same
    //binning in each sideband N, each sideband's chi2 has N-1 degrees of freedom.  So, they
    //are i.i.d. chi2 random variables whose sum is also chi2-distributed.
    double chi2 = 0;
 
    //fFirstBin and fLastBin should always be in the range [1, nBins]
    //because under/overflow bins should not be fit.
    for(int whichBin = fFirstBin; whichBin < fLastBin; ++whichBin)
    {
      std::vector<double> backgroundFitFuncs(fBackgrounds.size());
      const double binCenter = fSidebands.front().floatingHists.front()->GetXaxis()->GetBinCenter(whichBin);
      int whichParam = 0;
      for(size_t whichBackground = 0; whichBackground < fBackgrounds.size(); ++whichBackground)
      {
        backgroundFitFuncs[whichBackground] = fBackgrounds[whichBackground]->functionToFit(binCenter, parameters + whichParam);
        whichParam += fBackgrounds[whichBackground]->nPars();
      }
 
      for(const auto& sideband: fSidebands)
      {
        double floatingSum = 0,
               dataContent = sideband.data.GetBinContent(whichBin),
               dataErr = sideband.data.GetBinError(whichBin);
 
        for(size_t whichBackground = 0; whichBackground < fBackgrounds.size(); ++whichBackground)
        {
          floatingSum += sideband.floatingHists[whichBackground]->GetBinContent(whichBin) * backgroundFitFuncs[whichBackground];
        }
        //Don't add to chi2 if denominator is < 32-bit floating point precision.
        using namespace units;
        if(dataErr > 1e-10) chi2 += pow<2>((floatingSum + sideband.fixedSum->GetBinContent(whichBin))*fPOTScale - dataContent)/pow<2>(dataErr); //dataContent;
      }
    } //For each bin
 
    return fRegTerm?chi2 + fRegTerm->DoEval(parameters):chi2;
  } //Function call operator
 
  //Once the fit is complete, scale every histogram that was optimized based on the fit results
  void Universe::scale(Sideband& toModify, const ROOT::Math::Minimizer& fitParams) const
  {
    const double* parameters = fitParams.X();
    const double* errors = fitParams.Errors();
 
    int firstParam = 0;
    for(size_t whichBkg = 0; whichBkg < fBackgrounds.size(); ++whichBkg)
    {
      const auto bkg = fBackgrounds[whichBkg];
      for(int whichBin = fFirstBin; whichBin < fLastBin; ++whichBin)
      {
        const double binCenter = toModify.floatingHists[whichBkg]->GetXaxis()->GetBinCenter(whichBin);
        const double scaleFactor = bkg->functionToFit(binCenter, parameters + firstParam);
 
        auto floatHist = toModify.floatingHists[whichBkg];
        assert(std::string(floatHist->GetName()).find(bkg->name) != std::string::npos && "Background histogram and model name do not match!");
 
        const double oldContent = floatHist->GetBinContent(whichBin);
        floatHist->SetBinContent(whichBin, oldContent * scaleFactor);
 
        //Calculate uncertainty on scaled bin content from fit
        //Ignore the parameter correlation matrix because I shouldn't be using this fit anyway if the correlation matrix
        //has large off-diagonal elements.
        //N.B.: Doing this on non-CV universes unnecessarily increases their size on disk.  We currently don't use statistical
        //      uncertainty on universe histograms for anything.
        const double sqErrOnScaleFactor = bkg->getSqErrOnFunction(binCenter, parameters + firstParam, errors + firstParam),
                     errOnBin = floatHist->GetBinError(whichBin);
        using namespace units;
        floatHist->SetBinError(whichBin, std::sqrt(pow<2>(errOnBin) * pow<2>(scaleFactor) + sqErrOnScaleFactor * pow<2>(oldContent))); //Uncerainty on product of uncorrelated random variables
      } //For each bin
 
      firstParam += bkg->nPars();
    } //For each background
  } //scale()
 
  //Required for ROOT fittable function base class :(
  ROOT::Math::IBaseFunctionMultiDimTempl<double>* Universe::Clone() const 
  {
    return new Universe(fSidebands, fBackgrounds, fPOTScale, fRegTerm, fFirstBin, fLastBin);
  }
}
