//File: FitSidebands.cpp
//Brief: Given histograms from a CrossSection<> Study, use the data/MC ratio in each sideband to fit each background category.
//       This is a joint fit that is performed in each systematic Universe.  Currently a linear fit based on the data/MC ratios
//       in the multi-neutron sample.
//Usage: FitSidebands <data.root> <mc.root>
//Author: Andrew Olivier aolivier@ur.rochester.edu

//fits includes
#include "fits/Sideband.h"
#include "fits/Universe.h"
#include "fits/Background.h"

//util includes
#include "util/mathWithUnits.h"
#include "util/GetIngredient.h"
#include "util/Factory.cpp"

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

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

namespace
{
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

  std::vector<std::string> findBackgroundNames(TDirectoryFile& dir)
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

void plotParameterScan(ROOT::Math::Minimizer& minimizer, const fit::Universe& univ, const std::string& bandName, const int whichUniv, const std::vector<double>& CVResults, const std::vector<double>& CVErrors)
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

  if(argc != 4) //Remember that argv[0] is the executable name
  {
    std::cerr << "Expected exactly 3 arguments, but got " << argc-1 << "\n"
              << "USAGE: FitSidebands <config.yaml> <data.root> <mc.root>\n";
    return 1;
  }

  std::unique_ptr<TFile> dataFile(TFile::Open(argv[2], "READ"));
  if(!dataFile)
  {
    std::cerr << "Failed to open file with data histograms named " << argv[2] << "\n";
    return 2;
  }

  std::string mcBaseName = argv[3];
  mcBaseName = mcBaseName.substr(0, mcBaseName.find(".root")) + "_constrainedBy_" + argv[1];
  mcBaseName = mcBaseName.substr(0, mcBaseName.find(".yaml")) + ".root";
  if(gSystem->CopyFile(argv[3], mcBaseName.c_str()) != 0)
  {
    std::cerr << "Failed to copy file with MC histograms named " << argv[2] << "\n";
    return 3;
  }
  std::unique_ptr<TFile> mcFile(TFile::Open(mcBaseName.c_str(), "UPDATE"));

  YAML::Node config;
  try
  {
    config = YAML::LoadFile(argv[1]);
  }
  catch(const YAML::Exception& e)
  {
    std::cerr << "Failed to configure sideband fit from " << argv[1] << " because:\n" << e.what() << "\n";
    return 4;
  }

  //Configure this program using the input YAML file
  const std::string dummySelectionName = findSelectionName(*mcFile);
  const auto exampleHist = util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, dummySelectionName + "_SelectedMCEvents");

  bool fitToSelection = false;
  int firstBin = 1, lastBin = exampleHist->GetXaxis()->GetNbins() + 1;

  try
  {
    if(config["fitToSelection"]) fitToSelection = config["fitToSelection"].as<bool>();
    if(config["range"])
    {
      firstBin = config["range"]["firstBin"].as<int>(firstBin);
      lastBin = config["range"]["lastBin"].as<int>(lastBin);
    }
  }
  catch(const YAML::Exception& e)
  {
    std::cerr << "Failed to set up fitToSelection or fit range because:\n" << e.what() << "\n";
    return 5;
  }

  //fixedBackgroundNames is now inferred to be every Background that isn't listed in the YAML file
  //const auto fixedBackgroundNames = config["fixedBackgrounds"].as<std::vector<std::string>>({"Other"}); //{"Other", "ProtonsAboveAmitsThresholdOnly", "0_Neutrons", "MultiPi"}; //{"Other", "MultiPi", "0_Neutrons"};

  //Figure out where the rest of the histograms belong by context
  const double mcPOT = util::GetIngredient<TParameter<double>>(*mcFile, "POTUsed")->GetVal(),
               dataPOT = util::GetIngredient<TParameter<double>>(*dataFile, "POTUsed")->GetVal();

  //const auto crossSectionPrefixes = findCrossSectionPrefixes(*dataFile); //TODO: Maybe this is the longest unique string at the beginning of all keys?

  const auto allBackgrounds = findBackgroundNames(*mcFile);

  //Figure out sum of bin widths in fit region in case I want to use a linearly scaled background
  const double sumBinWidths = exampleHist->GetBinLowEdge(lastBin + 1) - exampleHist->GetBinLowEdge(firstBin);

  //TODO: YAML-ify this with Factory.  Perhaps hold all backgrounds not listed constant?
  std::vector<fit::Background*> backgrounds;
  std::vector<std::string> fixedBackgroundNames, backgroundsToFit;
  bool floatSignal = false;
  for(const auto& bkgName: allBackgrounds)
  {
    if(config["fits"][bkgName])
    {
      backgroundsToFit.push_back(bkgName);
      try
      {
        backgrounds.emplace_back(plgn::Factory<fit::Background, const std::string&, double>::instance().Get(config["fits"][bkgName], bkgName, sumBinWidths).release());
      }
      catch(const std::exception& e)
      {
        std::cerr << "Failed to configure a fit for a background named " << bkgName << " because of a YAML error:\n" << e.what() << "\n";
        return 5;
      }
    }
    else fixedBackgroundNames.push_back(bkgName);
  }
  if(config["fits"]["signal"]) //Treating the signal as something that can be fit needs to be handled specially because
                               //signal histograms annoyingly have a different naming convention.
  {
    floatSignal = true;
    backgrounds.emplace_back(plgn::Factory<fit::Background, const std::string&, double>::instance().Get(config["fits"]["signal"], "Signal", sumBinWidths).release());
  }

  //Program status to return to the operating system.  Nonzero indicates a problem that will stop
  //i.e. a cross section extraction script.  I'm going to keep going if an individual fit fails
  //but return a nonzero programStatus.
  int programStatus = 0;

  /*for(const auto& prefix: crossSectionPrefixes) //Usually a loop over fiducial volumes
  {*/
    const std::string selectionName = findSelectionName(*mcFile);
    /*const*/ std::vector<std::string> sidebandNames = findSidebandNames(*mcFile, selectionName);

    for(const auto nameToIgnore: config["ignoreSidebands"])
    {
      auto toRemove = std::remove_if(sidebandNames.begin(), sidebandNames.end(), [&nameToIgnore](const auto& name) { return name.find(nameToIgnore.as<std::string>()) != std::string::npos; });
      sidebandNames.erase(toRemove, sidebandNames.end());
    }

    //Fit the Central Value (CV) backgrounds.  These are the numbers actually subtracted from the data to make it a cross section.
    std::vector<fit::Sideband> cvSidebands;
    for(const auto& name: sidebandNames) cvSidebands.emplace_back(name, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, floatSignal);
    if(fitToSelection) cvSidebands.emplace_back(selectionName, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, floatSignal);
    fit::Universe objectiveFunction(cvSidebands, backgrounds, dataPOT/mcPOT, firstBin, lastBin);

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
    fit::Sideband cvSelection(selectionName, *dataFile, *mcFile, backgroundsToFit, fixedBackgroundNames, floatSignal);
    if(!fitToSelection) objectiveFunction.scale(cvSelection, *minimizer);

    //Get the list of error bands to loop over
    auto referenceHist = util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, selectionName + "_Signal");
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

        std::vector<fit::Sideband> sidebands;
        for(const auto& cvSideband: cvSidebands) sidebands.emplace_back(cvSideband, bandName, whichUniv);

        int nextPar = 0;
        for(auto bkg: backgrounds)
        {
          bkg->guessInitialParameters(*minimizer, nextPar, sidebands, dataPOT/mcPOT);
          nextPar += bkg->nPars();
        }

        fit::Universe objectiveFunction(sidebands, backgrounds, dataPOT/mcPOT, firstBin, lastBin);
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
          #ifndef NDEBUG
          plotParameterScan(*minimizer, objectiveFunction, bandName, whichUniv, CVResults, CVErrors);
          #endif //NDEBUG
        }

        for(auto& sideband: sidebands) objectiveFunction.scale(sideband, *minimizer);
        if(!fitToSelection)
        {
          fit::Sideband selection(cvSelection, bandName, whichUniv);
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
