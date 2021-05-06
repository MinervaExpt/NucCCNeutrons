//File: FitSidebands.cpp
//Brief: Given histograms from a CrossSection<> Study, use the data/MC ratio in each sideband to fit each background category.
//       This is a joint fit that is performed in each systematic Universe.  Currently a linear fit based on the data/MC ratios
//       in the multi-neutron sample.
//Usage: FitSidebands <data.root> <mc.root>
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Fit includes
#include "fit/Background.h"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"

//ROOT includes
#include "TH1.h"
#include "Math/IFunction.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "TSystem.h"
#include "TDirectoryFile.h"
#include "TFile.h"
#include "TParameter.h"
#include "Minuit2/Minuit2Minimizer.h" //TODO: Remove me?
#include "TMinuitMinimizer.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TCanvas.h"

//TODO: Share GetIngredient() and possibly the prefix and background search functions with ExtractCrossSection.
//      Put them in util or something.
namespace
{
  template <int power>
  double pow(const double base)
  {
    static_assert(power > 0, "Powers < 0 not yet implemented");
    return base * pow<power-1>(base);
  }

  template <>
  double pow<1>(const double base)
  {
    return base;
  }

  //Get a cross section ingredient histogram or TObject with a useful message on failure
  template <class TYPE>
  TYPE* GetIngredient(TDirectoryFile& dir, const std::string& ingredient)
  {
    TObject* obj = dir.Get(ingredient.c_str());
    if(obj == nullptr) throw std::runtime_error("Failed to get " + ingredient + " in " + dir.GetName());
  
    auto typed = dynamic_cast<TYPE*>(obj);
    if(typed == nullptr) throw std::runtime_error(std::string("Found ") + obj->GetName() + ", but it's not the right kind of TObject.");
  
    return typed;
  }

  template <class TYPE>
  TYPE* GetIngredient(TDirectoryFile& dir, const std::string& ingredient, const std::string& prefix)
  {
    return GetIngredient<TYPE>(dir, prefix + "_" + ingredient);
  }

  //Each background category in a single sideband for a single systematic Universe
  struct Sideband
  {
    //Construct a Sideband in the CV
    Sideband(const std::string& sidebandName, TDirectoryFile& dataDir, TDirectoryFile& mcDir, const std::vector<std::string>& floatingBackgroundNames, const std::vector<std::string>& fixedNames, const bool floatSignal = false)
    {
      //Unfortunately, the data in the selection region has a different naming convention.  It ends with "_Signal".
      //TODO: Just rename histograms in ExtractCrossSection so that data ends with "_Data" in selection region too.  "Signal" is a stupid and confusing name anyway.
      try
      { 
        data = GetIngredient<PlotUtils::MnvH1D>(dataDir, sidebandName + "_Data")->GetCVHistoWithStatError();  
      }
      catch(const std::runtime_error& e)
      { 
        data = GetIngredient<PlotUtils::MnvH1D>(dataDir, sidebandName + "_Signal")->GetCVHistoWithStatError();
      }

      for(const auto bkg: floatingBackgroundNames)
      {
        floatingHists.push_back(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + bkg));
      }

      fixedSum.reset(static_cast<PlotUtils::MnvH1D*>(floatingHists.front()->Clone()));
      fixedSum->Reset();
      for(const auto& fixed: fixedNames) fixedSum->Add(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + fixed));

      //Let signal float too
      try
      {
        if(!floatSignal) fixedSum->Add(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_TruthSignal"));
        else floatingHists.push_back(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_TruthSignal"));
      }
      catch(const std::runtime_error& e)
      {
        if(!floatSignal) fixedSum->Add(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_SelectedMCEvents"));
        else floatingHists.push_back(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_SelectedMCEvents"));
      }
    }

    //Construct a Sideband in a specific, non-CV, systematic universe using the CV sideband as the source of histograms.
    //This makes sure the sideband histograms' modifications are written to the output file with the CV.
    Sideband(const Sideband& cvSideband, const std::string& errorBandName, const int whichUniv): data(cvSideband.data), fixedSum(static_cast<TH1D*>(dynamic_cast<PlotUtils::MnvH1D*>(cvSideband.fixedSum.get())->GetVertErrorBand(errorBandName)->GetHist(whichUniv)->Clone()))
    {
      for(const auto cvHist: cvSideband.floatingHists)
      {
        floatingHists.push_back(dynamic_cast<PlotUtils::MnvH1D*>(cvHist)->GetVertErrorBand(errorBandName)->GetHist(whichUniv));
      }
    }

    //Observer pointers to histograms that came from (and will be deleted by) a TFile
    TH1D data;
    std::unique_ptr<TH1> fixedSum;
    std::vector<TH1D*> floatingHists;

    //Make the compiler happy.
    Sideband(const Sideband& parent): data(parent.data), fixedSum(static_cast<TH1D*>(parent.fixedSum->Clone())), floatingHists(parent.floatingHists)
    {
    }
  };

  //TODO: Put Background fit options in their own directory?
  struct Background
  {
    Background(const std::string& bkgdName): name(bkgdName) {}
    virtual ~Background() = default;

    //functionToFit() tries to model a data/MC ratio for a specific background in a sideband.
    //It takes the bin number and nPars() parameters from the fitter as arguments.
    virtual double functionToFit(const double binCenter, const double* pars) const = 0;
    virtual double getSqErrOnFunction(const double binCenter, const double* pars, const double* parErrs) const = 0;
    virtual int nPars() const = 0;

    virtual void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const = 0;

    std::string name; //This background scales all histograms with this string in their names

    //Helper functions for guessing parameter starting values
    protected:
      std::unique_ptr<TH1D> makeDataMCRatio(const Sideband& sideband, const double POTRatio) const
      {
        std::unique_ptr<TH1D> mcRatio(static_cast<TH1D*>(sideband.fixedSum->Clone()));
        for(const auto& hist: sideband.floatingHists) mcRatio->Add(hist);
        mcRatio->Scale(POTRatio);
        mcRatio->Divide(&sideband.data, mcRatio.get());

        return std::move(mcRatio);
      }

      auto getMostPureSideband(const std::vector<Sideband>& sidebands) const -> decltype(sidebands.begin())
      {
        assert(!sidebands.empty());

        const auto whichHist = std::find_if(sidebands.front().floatingHists.begin(), sidebands.front().floatingHists.end(), [this](const auto hist) { return std::string(hist->GetName()).find(this->name) != std::string::npos; });
        assert(whichHist != sidebands.front().floatingHists.end());
        const size_t index = std::distance(sidebands.front().floatingHists.begin(), whichHist);

        const auto largestSideband = std::max_element(sidebands.begin(), sidebands.end(),
                                                      [index](const auto& lhs, const auto& rhs)
                                                      {
                                                        const auto sumHists = [](const double sum, const auto hist) { return sum + hist->Integral(); };
                                                        const double lhsTotal = std::accumulate(lhs.floatingHists.begin(), lhs.floatingHists.end(), lhs.fixedSum->Integral(), sumHists);
                                                        const double rhsTotal = std::accumulate(rhs.floatingHists.begin(), rhs.floatingHists.end(), rhs.fixedSum->Integral(), sumHists);
                                                        return lhs.floatingHists[index]->Integral()/lhsTotal < rhs.floatingHists[index]->Integral()/rhsTotal;
                                                      });

        return largestSideband;
      }
  };

  //Scale a background's normalization only.  The simplest kind of sideband fit and the one most often used by MINERvA analyses.
  struct ScaledBackground: public Background
  {
    ScaledBackground(const std::string& name): Background(name) {}
    virtual ~ScaledBackground() = default;

    double functionToFit(const double /*binCenter*/, const double* pars) const override
    {
      return pars[0];
    }

    double getSqErrOnFunction(const double /*binCenter*/, const double* /*pars*/, const double* parErrs) const override
    {
      return pow<2>(parErrs[0]);
    }

    int nPars() const override { return 1; }

    void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const override
    {
      const auto largestSideband = getMostPureSideband(sidebands);
      assert(largestSideband != sidebands.end());

      auto mcRatio = makeDataMCRatio(*largestSideband, POTRatio);
      const double scaleGuess = (mcRatio->GetMaximum() - mcRatio->GetMinimum())/2. + mcRatio->GetMinimum();
      #ifndef NDEBUG
      std::cout << "Setting guess for scaled background " << name << " (index = " << nextPar << ") to " << scaleGuess << "\n"
                << "Ratio max is " << mcRatio->GetMaximum() << "\nRatio min is " << mcRatio->GetMinimum() << "\n";
      #endif //NDEBUG

      //min.SetLimitedVariable(nextPar, name.c_str(), scaleGuess, scaleGuess/20., mcRatio->GetMinimum(), mcRatio->GetMaximum());
      min.SetVariable(nextPar, name.c_str(), scaleGuess, scaleGuess/20.);
      min.SetVariableLimits(nextPar, std::min(0.5, scaleGuess), std::max(2., scaleGuess)); //1.5);
    }
  };

  //Fit a line to a sideband's data/MC ratio
  struct LinearBackground: public Background
  {
    LinearBackground(const std::string& name, const double sumBinWidths): Background(name), fSumBinWidths(sumBinWidths) {}
    virtual ~LinearBackground() = default;

    double functionToFit(const double binCenter, const double* pars) const override
    {
      const double slope = (pars[1] - pars[0]) / fSumBinWidths;
      return pars[0] + slope * binCenter;
    }

    double getSqErrOnFunction(const double binCenter, const double* /*pars*/, const double* parErrs) const override
    {
      //Assuming no correlation between slope and intercept parameters or any other Backgrounds
      return pow<2>(parErrs[1] * binCenter / fSumBinWidths) + pow<2>((binCenter/fSumBinWidths + 1) * parErrs[0]);
      //return pow<2>(binCenter * parErrs[1]) + pow<2>(parErrs[0]);  //Old version was wrong!
    }

    int nPars() const override { return 2; }

    void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const override
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

    private:
      const double fSumBinWidths;
  };

  //Based heavily on code by Aaron Bercellie at Ana/NukeCCPion/ana/ChainWrapper/src/SidebandFitter.cxx.  It's not on CVS though as of March 27, 2021.
  class Universe: public ROOT::Math::IBaseFunctionMultiDimTempl<double>
  {
    public:
      Universe(const std::vector<Sideband>& sidebands, std::vector<Background*> backgrounds, const double POTRatio,
               const int firstBin = 1, const int lastBin = -1): IBaseFunctionMultiDimTempl<double>(), fSidebands(sidebands), fBackgrounds(backgrounds), fPOTScale(POTRatio), fFirstBin(firstBin), fLastBin(lastBin)
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

      unsigned int NDim() const override
      {
        return std::accumulate(fBackgrounds.begin(), fBackgrounds.end(), 0l, [](const int sum, const Background* bkg) { return sum + bkg->nPars(); });
      }

      //Chi squared objective function that some ROOT fitter will optimize in parameters
      double DoEval(const double* parameters) const override
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
            if(dataErr > 1e-10) chi2 += pow<2>((floatingSum + sideband.fixedSum->GetBinContent(whichBin))*fPOTScale - dataContent)/pow<2>(dataErr); //dataContent;
          }
        } //For each bin

        return chi2;
      } //Function call operator

      //Once the fit is complete, scale every histogram that was optimized based on the fit results
      void scale(Sideband& toModify, const ROOT::Math::Minimizer& fitParams) const
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
            floatHist->SetBinError(whichBin, std::sqrt(pow<2>(errOnBin) * pow<2>(scaleFactor) + sqErrOnScaleFactor * pow<2>(oldContent))); //Uncerainty on product of uncorrelated random variables
          } //For each bin

          firstParam += bkg->nPars();
        } //For each background
      } //scale()

      //Required for ROOT fittable function base class :(
      IBaseFunctionMultiDimTempl<double>* Clone() const override
      {
        return new Universe(fSidebands, fBackgrounds, fPOTScale, fFirstBin);
      }

    private:
      std::vector<Sideband> fSidebands;
      std::vector<Background*> fBackgrounds; //Observer pointers

      double fPOTScale;
      int fFirstBin; //Bin to start sideband fit
      int fLastBin; //Last bin included in sideband fit
  };

  std::string findSelectionName(TDirectoryFile& dir)
  {
    const std::string selectionTag = "_Signal";
    for(auto key: *dir.GetListOfKeys())
    {
      std::string keyName = key->GetName();
      const size_t selectionLocation = keyName.find(selectionTag);
      if(selectionLocation != std::string::npos) return keyName.substr(0, selectionLocation);
    }

    throw std::runtime_error("Failed to find a histogram whose name ends in " + selectionTag + " in " + dir.GetName());
    return "";
  }

  std::vector<std::string> findSidebandNames(TDirectoryFile& dir, const std::string& selectionName)
  {
    std::set<std::string> uniquePrefixes;
    for(auto key: *dir.GetListOfKeys())
    {
      std::string keyName = key->GetName();
      const size_t bkgLocation = keyName.find("_Background_");
      if(bkgLocation != std::string::npos && keyName.find(selectionName) == std::string::npos) uniquePrefixes.insert(keyName.substr(0, bkgLocation));
    }

    return std::vector<std::string>(uniquePrefixes.begin(), uniquePrefixes.end());
  }

  std::vector<std::string> findBackgroundNames(TDirectoryFile& dir, const std::vector<std::string>& fixedBackgrounds)
  {
    const std::string tag = "_Background_";
    std::set<std::string> uniqueBackgrounds;
    for(auto key: *dir.GetListOfKeys())
    {
      std::string keyName = key->GetName();
      const size_t bkgLocation = keyName.find(tag);
      if(bkgLocation != std::string::npos)
      {
        const std::string bkgName = keyName.substr(bkgLocation + tag.length(), std::string::npos);
        if(std::find(fixedBackgrounds.begin(), fixedBackgrounds.end(), bkgName) == fixedBackgrounds.end())
          uniqueBackgrounds.insert(bkgName);
      }
    }

    return std::vector<std::string>(uniqueBackgrounds.begin(), uniqueBackgrounds.end());
  }
}

bool checkCovMatrixOffDiagonal(const ROOT::Math::Minimizer& minim, const int nPars)
{
  std::vector<double> covMatrix(nPars * nPars, 0);
  minim.GetCovMatrix(covMatrix.data());

  for(int xPar = 0; xPar < nPars; ++xPar)
  {
    for(int yPar = 0; yPar < nPars; ++yPar)
    {
      if(xPar == yPar) continue; //Ignore variance on individual parameters
      if(covMatrix[xPar * nPars + yPar] > 0.1) return false;
    }
  }

  return true;
}

void printCorrMatrix(const ROOT::Math::Minimizer& minim, const int nPars)
{
  std::vector<double> covMatrix(nPars * nPars, 0);
  minim.GetCovMatrix(covMatrix.data());
  const double* errors = minim.Errors();

  for(int xPar = 0; xPar < nPars; ++xPar)
  {
    std::cout << "[";
    for(int yPar = 0; yPar < nPars-1; ++yPar)
    { 
      std::cout << std::fixed << std::setprecision(2) << std::setw(5) << covMatrix[xPar * nPars + yPar]/errors[xPar]/errors[yPar] << ", ";
    }
    std::cout << std::fixed << std::setprecision(2) << std::setw(5) << covMatrix[xPar * nPars + nPars-1]/errors[xPar]/errors[nPars-1] << "]\n";
  }
}

void plotParameterScan(ROOT::Math::Minimizer& minimizer, const Universe& univ, const std::string& bandName, const int whichUniv, const std::vector<double>& CVResults, const std::vector<double>& CVErrors)
{
  constexpr unsigned int nStepsMax = 20; //1000
  const unsigned int nDim = univ.NDim();

  //Let the user see a graph of chi2 values around the minimum chi2.
  //Useful for debugging suspected bad fits caused by other local minima.
  //Documentation indicates that this may only work with Minuit-based minimizers.  So not the GSL minimizer.
  const double* bandResults = minimizer.X();
  const double* bandErrors = minimizer.Errors();

  //1D scan in each parameter
  std::vector<double> parMins(nDim), parMaxes(nDim);
  for(unsigned int whichPar = 0; whichPar < nDim; ++whichPar)
  {
    unsigned int nSteps = nStepsMax;
    double parValues[nStepsMax], chi2Values[nStepsMax];
    parMins[whichPar] = std::min(bandResults[whichPar] - 5*bandErrors[whichPar], CVResults[whichPar] - 5*CVErrors[whichPar]);
    parMaxes[whichPar] = std::max(bandResults[whichPar] + 5*bandErrors[whichPar], CVResults[whichPar] + 5*CVErrors[whichPar]);
    minimizer.Scan(whichPar, nSteps, parValues, chi2Values, parMins[whichPar], parMaxes[whichPar]);

    TGraph graph(nSteps, parValues, chi2Values);
    const std::string canName = bandName + "_universe_" + std::to_string(whichUniv) + "_parameter_" + std::to_string(whichPar) + "_chi2Scan";
    TCanvas canvas(canName.c_str(), "{#chi}^{2} Scan");

    graph.Draw();
    canvas.Print((canName + ".png").c_str());
  }

  //2D scan in all pairs of parameters
  for(unsigned int whichFirstPar = 0; whichFirstPar < nDim; ++whichFirstPar)
  {
    for(unsigned int whichSecondPar = whichFirstPar + 1; whichSecondPar < nDim; ++whichSecondPar)
    {
      TGraph2D graph(nStepsMax * nStepsMax);
      for(unsigned int firstParStep = 0; firstParStep < nStepsMax; ++firstParStep)
      {
        for(unsigned int secondParStep = 0; secondParStep < nStepsMax; ++secondParStep)
        {
          const double firstPar = parMins[whichFirstPar] + firstParStep * (parMaxes[whichFirstPar] - parMins[whichFirstPar]) / nStepsMax,
                       secondPar = parMins[whichSecondPar] + secondParStep * (parMaxes[whichSecondPar] - parMins[whichSecondPar]) / nStepsMax;
          std::vector<double> allPars(minimizer.X(), minimizer.X() + nDim);
          allPars[whichFirstPar] = firstPar;
          allPars[whichSecondPar] = secondPar;

          graph.SetPoint(firstParStep * nStepsMax + secondParStep, firstPar, secondPar, univ(allPars.data()));
        }
      }

      std::string canName = bandName + "_universe_" + std::to_string(whichUniv) + "_parameters_" + std::to_string(whichFirstPar) + "_" + std::to_string(whichSecondPar) + "_chi2Scan";
      TCanvas canvas(canName.c_str(), "{#chi}^{2} Scan");
      graph.Draw("surf1");
      canvas.Print((canName + ".png").c_str());
    }
  }
}

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(kFALSE); //Don't add any temporary histograms to the output file by default.  Let me delete them myself.
  gErrorIgnoreLevel = kWarning; //Silence TCanvas::Print()

  const bool floatSignal = true,
             fitToSelection = true;
  const int firstBin = 1,
            lastBin = 19;

  if(argc != 3) //Remember that argv[0] is the executable name
  {
    std::cerr << "Expected exactly 2 arguments, but got " << argc-1 << "\n"
              << "USAGE: FitSidebands <data.root> <mc.root>\n";
    return 1;
  }

  auto dataFile = TFile::Open(argv[1], "READ");
  if(!dataFile)
  {
    std::cerr << "Failed to open file with data histograms named " << argv[1] << "\n";
    return 2;
  }

  std::string mcBaseName = argv[2];
  mcBaseName = mcBaseName.substr(0, mcBaseName.find(".root"));
  if(gSystem->CopyFile(argv[2], (mcBaseName + "_constrained.root").c_str()) != 0)
  {
    std::cerr << "Failed to copy file with MC histograms named " << argv[2] << "\n";
    return 3;
  }
  auto mcFile = TFile::Open((mcBaseName + "_constrained.root").c_str(), "UPDATE");

  const double mcPOT = GetIngredient<TParameter<double>>(*mcFile, "POTUsed")->GetVal(),
               dataPOT = GetIngredient<TParameter<double>>(*dataFile, "POTUsed")->GetVal();

  //const auto crossSectionPrefixes = findCrossSectionPrefixes(*dataFile); //TODO: Maybe this is the longest unique string at the beginning of all keys?

  const std::vector<std::string> fixedBackgroundNames = {"Other", "ProtonsAboveAmitsThresholdOnly", "0_Neutrons", "MultiPi"}; //{"Other", "MultiPi", "0_Neutrons"};
  const auto backgroundsToFit = findBackgroundNames(*mcFile, fixedBackgroundNames);

  //Figure out sum of bin widths in fit region in case I want to use a linearly scaled background
  const std::string dummySelectionName = findSelectionName(*mcFile);
  const auto exampleHist = GetIngredient<PlotUtils::MnvH1D>(*mcFile, dummySelectionName + "_SelectedMCEvents");
  const double sumBinWidths = exampleHist->GetBinLowEdge(lastBin + 1) - exampleHist->GetBinLowEdge(firstBin);

  std::vector<Background*> backgrounds;
  for(const auto& bkgName: backgroundsToFit) backgrounds.push_back(new LinearBackground(bkgName, sumBinWidths)); //ScaledBackground(bkgName));
  if(floatSignal) backgrounds.push_back(new ScaledBackground("Signal"));

  //Program status to return to the operating system.  Nonzero indicates a problem that will stop
  //i.e. a cross section extraction script.  I'm going to keep going if an individual fit fails
  //but return a nonzero programStatus.
  int programStatus = 0;

  /*for(const auto& prefix: crossSectionPrefixes) //Usually a loop over fiducial volumes
  {*/
    const std::string selectionName = findSelectionName(*mcFile);
    /*const*/ std::vector<std::string> sidebandNames = findSidebandNames(*mcFile, selectionName);

    //Don't include the multi-pi sideband in the fit at all.  I shouldn't need it because its background is insignificant in the selection region.
    //This sideband isn't very pure in MultiPi backgrounds anyway.
    //TODO: Just stop including this sideband in the event loop
    auto toRemove = std::remove_if(sidebandNames.begin(), sidebandNames.end(), [](const auto& name) { return name.find("MultiPi") != std::string::npos; });
    sidebandNames.erase(toRemove, sidebandNames.end());

    //Fit the Central Value (CV) backgrounds.  These are the numbers actually subtracted from the data to make it a cross section.
    std::vector<Sideband> cvSidebands;
    for(const auto& name: sidebandNames) cvSidebands.emplace_back(name, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, floatSignal);
    if(fitToSelection) cvSidebands.emplace_back(selectionName, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, floatSignal);
    Universe objectiveFunction(cvSidebands, backgrounds, dataPOT/mcPOT, firstBin, lastBin);

    auto* minimizer = new ROOT::Minuit2::Minuit2Minimizer(ROOT::Minuit2::kMigrad); //TMinuitMinimizer(ROOT::Minuit::kMigrad, objectiveFunction.NDim()); //ROOT::Minuit2::Minuit2Minimizer(ROOT::Minuit2::kSimplex);

    int nextPar = 0;
    for(auto bkg: backgrounds)
    {
      bkg->guessInitialParameters(*minimizer, nextPar, cvSidebands, dataPOT/mcPOT);
      nextPar += bkg->nPars();
    }

    assert(nextPar == objectiveFunction.NDim());
    minimizer->SetFunction(objectiveFunction);
    if(!minimizer->Minimize()) programStatus = 4;

    std::cout << "CV fit results.  Universe fit results will only be printed if the minimization fails or when compiled in debug mode (NDEBUG not defined).\n";
    minimizer->PrintResults();
    std::cout << "\nCV Correlation Matrix\n";
    printCorrMatrix(*minimizer, objectiveFunction.NDim());

    //Take a copy of CV fit results before they get overridden!
    const double cvChi2 = minimizer->MinValue();
    std::vector<double> CVResults(minimizer->X(), minimizer->X() + objectiveFunction.NDim()),
                        CVErrors(minimizer->Errors(), minimizer->Errors() + objectiveFunction.NDim());

    for(auto& sideband: cvSidebands) objectiveFunction.scale(sideband, *minimizer);
    Sideband cvSelection(selectionName, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, floatSignal);
    if(!fitToSelection) objectiveFunction.scale(cvSelection, *minimizer);

    //Get the list of error bands to loop over
    auto referenceHist = GetIngredient<PlotUtils::MnvH1D>(*mcFile, selectionName + "_Signal");
    const auto errorBandNames = referenceHist->GetErrorBandNames();

    //Fit each error band to the data.  This really constrains the uncertainty on the simulated backgrounds.
    for(const auto& bandName: errorBandNames)
    {
      const auto univs = referenceHist->GetVertErrorBand(bandName)->GetHists();
      for(size_t whichUniv = 0; whichUniv < univs.size(); ++whichUniv)
      {
        #ifndef NDEBUG
        std::cout << "Fitting error band " << bandName << " universe " << whichUniv << ".\n";
        #endif //NDEBUG

        std::vector<Sideband> sidebands;
        for(const auto& cvSideband: cvSidebands) sidebands.emplace_back(cvSideband, bandName, whichUniv);

        int nextPar = 0;
        for(auto bkg: backgrounds)
        {
          bkg->guessInitialParameters(*minimizer, nextPar, sidebands, dataPOT/mcPOT);
          nextPar += bkg->nPars();
        }

        Universe objectiveFunction(sidebands, backgrounds, dataPOT/mcPOT, firstBin, lastBin);
        assert(nextPar == objectiveFunction.NDim());
        minimizer->SetFunction(objectiveFunction);

        bool printBandInfo = false;
        #ifndef NDEBUG
        printBandInfo = true;
        #endif //NDEBUG

        if(minimizer->Minimize() == false)
        {
          std::cerr << "Fit failed for universe " << whichUniv << " in error band " << bandName
                    << "!  I'm going to save this result and keep going with the other universes anyway...\n";
          printBandInfo = true;
          programStatus = 4;
        }
        else if(minimizer->MinValue() > 1.5*cvChi2) //TODO: Add other conditions here like scanning the correlation matrix
        {
          std::cerr << "Fit succeeded but warrants further study.\n";
          printBandInfo = true;
        }

        if(printBandInfo)
        {
          std::cout << bandName << " error band universe " << whichUniv << "\n";
          minimizer->PrintResults();
          std::cout << "\nCorrelation matrix:\n";
          printCorrMatrix(*minimizer, objectiveFunction.NDim());
          plotParameterScan(*minimizer, objectiveFunction, bandName, whichUniv, CVResults, CVErrors);
        }

        for(auto& sideband: sidebands) objectiveFunction.scale(sideband, *minimizer);
        if(!fitToSelection)
        {
          Sideband selection(cvSelection, bandName, whichUniv);
          objectiveFunction.scale(selection, *minimizer);
        }

        //TODO: Propagate errors from TMinuit
        //TODO: Print fit diagnostics.  Throw out bad fits if I can come up with a robust scheme to detect some of them.
      } //Loop over universes

      //An MnvH1D actually contains many copies of the CV, one for each MnvVertErrorBand, that are not
      //explicitly synchronized.  I changed the CV when I fit it, so I need to synchronize the error bands
      //by hand.  If I don't do this, the systematic error bars MnvH1D calculates (really the covariance matrix)
      //will be wrong.
      for(const auto& cvSideband: cvSidebands)
      {
        for(const auto cvHist: cvSideband.floatingHists)
        {
          auto histToUpdate = dynamic_cast<PlotUtils::MnvH1D*>(cvHist);
          histToUpdate->GetVertErrorBand(bandName)->TH1D::operator=(*cvHist);
        }
      }
      if(!fitToSelection)
      {
        for(const auto cvHist: cvSelection.floatingHists)
        {
          auto histToUpdate = dynamic_cast<PlotUtils::MnvH1D*>(cvHist);
          histToUpdate->GetVertErrorBand(bandName)->TH1D::operator=(*cvHist);
        }
      }
    } //Loop over error bands

    //Make sure updated histograms end up in the output file
    mcFile->cd();
    for(const auto& cvSideband: cvSidebands)
    {
      for(const auto cvHist: cvSideband.floatingHists) cvHist->Write("", TObject::kOverwrite);
    }
    if(!fitToSelection) for(const auto cvHist: cvSelection.floatingHists) cvHist->Write("", TObject::kOverwrite);
  //} //Loop over fiducial volumes

  //TODO: Can I make sure I preserve the order of keys in the file somehow?  Keys that didn't get updated are ending up at the end of the directory list.
  //      The consequence is that my automatic color scheme changes.  I could probably just reorder the Backgrounds during the event loop if it comes to that.

  return programStatus;
}
