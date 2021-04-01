//File: FitSidebands.cpp
//Brief: Given histograms from a CrossSection<> Study, use the data/MC ratio in each sideband to fit each background category.
//       This is a joint fit that is performed in each systematic Universe.  Currently a linear fit based on the data/MC ratios
//       in the multi-neutron sample.
//Usage: FitSidebands <data.root> <mc.root>
//Author: Andrew Olivier aolivier@ur.rochester.edu

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
//#include "Minuit2/Minuit2Minimizer.h" //TODO: Remove me?
#include "TMinuitMinimizer.h"

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
    Sideband(const std::string& sidebandName, TDirectoryFile& dataDir, TDirectoryFile& mcDir, const std::vector<std::string>& floatingBackgroundNames, const std::vector<std::string>& fixedNames)
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

      fixedSum.reset(static_cast<TH1D*>(data.Clone()));
      fixedSum->Reset();

      for(const auto bkg: floatingBackgroundNames)
      {
        floatingHists.push_back(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + bkg));
      }
      for(const auto& fixed: fixedNames) fixedSum->Add(GetIngredient<TH1D>(mcDir, sidebandName + "_Background_" + fixed));

      //Keep signal contamination fixed too
      try
      {
        fixedSum->Add(GetIngredient<TH1>(mcDir, sidebandName + "_TruthSignal"));
      }
      catch(const std::runtime_error& e)
      {
        fixedSum->Add(GetIngredient<TH1>(mcDir, sidebandName + "_SelectedMCEvents"));
      }
    }

    //Construct a Sideband in a specific (non-CV) systematic universe
    Sideband(const std::string& sidebandName, TDirectoryFile& dataDir, TDirectoryFile& mcDir, const std::vector<std::string>& floatingBkgNames, const std::vector<std::string>& fixedNames, const std::string& errorBandName, const int whichUniv)
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

      fixedSum.reset(static_cast<TH1D*>(data.Clone()));
      fixedSum->Reset();

      for(const auto bkg: floatingBkgNames)
      {
        floatingHists.push_back(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + bkg)->GetVertErrorBand(errorBandName)->GetHist(whichUniv));
      }
      for(const auto& fixed: fixedNames) fixedSum->Add(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + fixed)->GetVertErrorBand(errorBandName)->GetHist(whichUniv));

      //Keep signal contamination fixed too
      try
      {
        fixedSum->Add(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_TruthSignal")->GetVertErrorBand(errorBandName)->GetHist(whichUniv));
      }
      catch(const std::runtime_error& e)
      {
        fixedSum->Add(GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_SelectedMCEvents")->GetVertErrorBand(errorBandName)->GetHist(whichUniv));
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
    virtual int nPars() const = 0;

    virtual void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const = 0;

    std::string name; //This background scales all histograms with this string in their names
  };

  //Scale a background's normalization only.  The simplest kind of sideband fit and the one most often used by MINERvA analyses.
  struct ScaledBackground: public Background
  {
    ScaledBackground(const std::string& name): Background(name) {}
    virtual ~ScaledBackground() = default;

    double functionToFit(const double /*binCenter*/, const double* pars) const override { return pars[0]; }
    int nPars() const override { return 1; }

    void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& sidebands, const double POTRatio) const override
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
      assert(largestSideband != sidebands.end());

      std::unique_ptr<TH1D> mcRatio(static_cast<TH1D*>(largestSideband->fixedSum->Clone()));
      for(const auto& hist: largestSideband->floatingHists) mcRatio->Add(hist);
      mcRatio->Scale(POTRatio);
      mcRatio->Divide(&largestSideband->data, mcRatio.get());
      const double scaleGuess = (mcRatio->GetMaximum() - mcRatio->GetMinimum())/2. + mcRatio->GetMinimum();
      std::cout << "Setting guess for scaled background " << name << " (index = " << index << ") to " << scaleGuess << "\n"
                << "Ratio max is " << mcRatio->GetMaximum() << "\nRatio min is " << mcRatio->GetMinimum() << "\n";

      //min.SetLimitedVariable(nextPar, name.c_str(), scaleGuess, scaleGuess/20., mcRatio->GetMinimum(), mcRatio->GetMaximum());
      min.SetVariable(nextPar, name.c_str(), scaleGuess, scaleGuess/20.);
    }
  };

  //Fit a line to a sideband's data/MC ratio
  struct LinearBackground: public Background
  {
    LinearBackground(const std::string& name): Background(name) {}
    virtual ~LinearBackground() = default;

    double functionToFit(const double binCenter, const double* pars) const override
    {
      return pars[0] + pars[1] * binCenter;
    }

    int nPars() const override { return 2; }

    void guessInitialParameters(ROOT::Math::Minimizer& min, const int nextPar, const std::vector<Sideband>& /*sidebands*/, const double /*POTRatio*/) const override
    {
      //TODO: Guess parameters based on sidebands
      min.SetVariable(nextPar, (name + " intercept").c_str(), 0, 1e-3);
      min.SetVariable(nextPar + 1, (name + " slope").c_str(), 0, 1e-3);
    }
  };

  //Based heavily on code by Aaron Bercellie at Ana/NukeCCPion/ana/ChainWrapper/src/SidebandFitter.cxx.  It's not on CVS though as of March 27, 2021.
  class Universe: public ROOT::Math::IBaseFunctionMultiDimTempl<double>
  {
    public:
      Universe(const std::vector<Sideband>& sidebands, std::vector<Background*> backgrounds, const double POTRatio,
               const int firstBin = 1): IBaseFunctionMultiDimTempl<double>(), fSidebands(sidebands), fBackgrounds(backgrounds), fPOTScale(POTRatio), fFirstBin(firstBin)
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
        fLastBin = fSidebands.front().data.GetXaxis()->GetNbins();
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
              floatingSum += sideband.floatingHists[whichBackground]->GetBinContent(whichBin) * backgroundFitFuncs[whichBin];
            }
            chi2 += pow<2>((floatingSum + sideband.fixedSum->GetBinContent(whichBin))*fPOTScale - dataContent)/pow<2>(dataErr); //TODO: Aaron divides by data error instead.  Seems like I've heard of this before, but I don't remember where or when to use it.  I saw it referred to as the reduce chi2 somewhere yesterday which I know means chi2/NDOF.  Otherwise, I divide by dataContent.
          }
        } //For each bin

        return chi2;
      } //Function call operator

      //Once the fit is complete, scale every histogram that was optimized based on the fit results
      void scale(Sideband& toModify, const ROOT::Math::Minimizer& fitParams) const
      {
        const double* parameters = fitParams.X();

        int firstParam = 0;
        for(size_t whichBkg = 0; whichBkg < fBackgrounds.size(); ++whichBkg)
        {
          const auto bkg = fBackgrounds[whichBkg];
          for(int whichBin = fFirstBin; whichBin < fLastBin; ++whichBin)
          {
            const double scaleFactor = bkg->functionToFit(toModify.floatingHists[whichBkg]->GetXaxis()->GetBinCenter(whichBin), parameters + firstParam);

            auto floatHist = toModify.floatingHists[whichBkg];
            assert(std::string(floatHist->GetName()).find(bkg->name) != std::string::npos && "Background histogram and model name do not match!");
            floatHist->SetBinContent(whichBin, floatHist->GetBinContent(whichBin) * scaleFactor);
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

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(kFALSE); //Don't add any temporary histograms to the output file by default.  Let me delete them myself.

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

  const std::vector<std::string> fixedBackgroundNames = {"Other", "MultiPi", "0_Neutrons"};
  const auto backgroundsToFit = findBackgroundNames(*mcFile, fixedBackgroundNames);

  std::vector<Background*> backgrounds;
  for(const auto& bkgName: backgroundsToFit) backgrounds.push_back(new ScaledBackground(bkgName));

  /*for(const auto& prefix: crossSectionPrefixes) //Usually a loop over fiducial volumes
  {*/
    auto* minimizer = new TMinuitMinimizer(ROOT::Minuit::kSimplex, 3); //Minuit2::Minuit2Minimizer(ROOT::Minuit2::kSimplex);

    //TODO: Overriding parameters by eye
    //For scale factor fit
    /*minimizer->SetLimitedVariable(0, "1_Neutron", 1.2, 5e-2, 1, 1.5);
    minimizer->SetVariable(1, "ChargedPions", 0.85, 5e-2);
    minimizer->SetVariable(2, "NeutralPionsOnly", 1, 5e-2);*/

    const std::string selectionName = findSelectionName(*mcFile);
    /*const*/ auto sidebandNames = findSidebandNames(*mcFile, selectionName); //TODO: When I can figure out the fiducial volume, don't include it in the sidebandNames

    //Don't include the multi-pi sideband in the fit at all.  I shouldn't need it because its background is insignificant in the selection region.
    //This sideband isn't very pure in MultiPi backgrounds anyway.
    //TODO: Just stop including this sideband in the event loop
    auto toRemove = std::remove_if(sidebandNames.begin(), sidebandNames.end(), [](const auto& name) { return name.find("MultiPi") != std::string::npos; });
    sidebandNames.erase(toRemove, sidebandNames.end());

    //Fit the Central Value (CV) backgrounds.  These are the numbers actually subtracted from the data to make it a cross section.
    std::vector<Sideband> cvSidebands;
    for(const auto& name: sidebandNames) cvSidebands.emplace_back(name, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames);

    int nextPar = 0;
    for(auto bkg: backgrounds)
    {
      bkg->guessInitialParameters(*minimizer, nextPar, cvSidebands, dataPOT/mcPOT);
      nextPar += bkg->nPars();
    }
    minimizer->SetVariableLimits(0, 0.9, 1.6); //Manually set limits on QELike sideband because it goes negative or very large

    Universe objectiveFunction(cvSidebands, backgrounds, dataPOT/mcPOT);
    assert(nextPar == objectiveFunction.NDim());
    minimizer->SetFunction(objectiveFunction);
    minimizer->Minimize();

    minimizer->PrintResults(); //TODO: Don't do this for every sideband.  Maybe just for the CV and sidebands with minimizer errors?  Looks like I can check the return code of minimize()

    for(auto& sideband: cvSidebands) objectiveFunction.scale(sideband, *minimizer);
    Sideband cvSelection(selectionName, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames);
    objectiveFunction.scale(cvSelection, *minimizer);

    //Get the list of error bands to loop over
    auto referenceHist = GetIngredient<PlotUtils::MnvH1D>(*mcFile, selectionName + "_Signal");
    const auto errorBandNames = referenceHist->GetErrorBandNames();

    //Fit each error band to the data.  This really constrains the uncertainty on the simulated backgrounds.
    for(const auto& bandName: errorBandNames)
    {
      const auto univs = referenceHist->GetVertErrorBand(bandName)->GetHists();
      for(size_t whichUniv = 0; whichUniv < univs.size(); ++whichUniv)
      {
        //TODO: Overriding parameters by eye
        //For scale factor fit
        /*minimizer->SetLimitedVariable(0, "1_Neutron", 1.2, 5e-2, 1, 1.5);
        minimizer->SetVariable(1, "ChargedPions", 0.85, 5e-2);
        minimizer->SetVariable(2, "NeutralPionsOnly", 1, 5e-2);*/

        std::cout << "Fitting error band " << bandName << " universe " << whichUniv << ".\n";
        std::vector<Sideband> sidebands;
        for(const auto& name: sidebandNames) sidebands.emplace_back(name, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, bandName, whichUniv);

        int nextPar = 0;
        for(auto bkg: backgrounds)
        {
          bkg->guessInitialParameters(*minimizer, nextPar, sidebands, dataPOT/mcPOT);
          nextPar += bkg->nPars();
        }
        minimizer->SetVariableLimits(0, 0.9, 1.6); //Manually set limits on QELike sideband because it goes negative or very large

        Universe objectiveFunction(sidebands, backgrounds, dataPOT/mcPOT);
        assert(nextPar == objectiveFunction.NDim());
        minimizer->SetFunction(objectiveFunction);
        minimizer->Minimize();

        minimizer->PrintResults(); //TODO: Don't do this for every sideband.  Maybe just for the CV and sidebands with minimizer errors?  Looks like I can check the return code of minimize()

        for(auto& sideband: sidebands) objectiveFunction.scale(sideband, *minimizer);
        Sideband selection(selectionName, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, bandName, whichUniv);
        objectiveFunction.scale(selection, *minimizer);

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
      for(const auto cvHist: cvSelection.floatingHists)
      {
        auto histToUpdate = dynamic_cast<PlotUtils::MnvH1D*>(cvHist);
        histToUpdate->GetVertErrorBand(bandName)->TH1D::operator=(*cvHist);
      }
    } //Loop over error bands

    //Make sure updated histograms end up in the output file
    mcFile->cd();
    for(const auto& cvSideband: cvSidebands)
    {
      for(const auto cvHist: cvSideband.floatingHists) cvHist->Write("", TObject::kOverwrite);
    }
    for(const auto cvHist: cvSelection.floatingHists) cvHist->Write("", TObject::kOverwrite);
  //} //Loop over fiducial volumes

  //TODO: Can I make sure I preserve the order of keys in the file somehow?  Keys that didn't get updated are ending up at the end of the directory list.
  //      The consequence is that my automatic color scheme changes.  I could probably just reorder the Backgrounds during the event loop if it comes to that.

  return 0;
}
