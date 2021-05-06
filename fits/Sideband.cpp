//File: Sideband.cpp
//Brief: A Sideband keeps track of all of the Backgrounds in a single sideband for a single systematic Universe.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//fit includes
#include "fits/Sideband.h"

//util includes
#include "util/GetIngredient.h"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"

//c++ includes
#include <exception>

namespace fit
{
  //Construct a Sideband in the CV
  Sideband::Sideband(const std::string& sidebandName, TDirectoryFile& dataDir, TDirectoryFile& mcDir, const std::vector<std::string>& floatingBackgroundNames, const std::vector<std::string>& fixedNames, const bool floatSignal)
  {
    //Unfortunately, the data in the selection region has a different naming convention.  It ends with "_Signal".
    //TODO: Just rename histograms in ExtractCrossSection so that data ends with "_Data" in selection region too.  "Signal" is a stupid and confusing name anyway.
    try
    { 
      data = util::GetIngredient<PlotUtils::MnvH1D>(dataDir, sidebandName + "_Data")->GetCVHistoWithStatError();  
    }
    catch(const std::runtime_error& e)
    { 
      data = util::GetIngredient<PlotUtils::MnvH1D>(dataDir, sidebandName + "_Signal")->GetCVHistoWithStatError();
    }

    for(const auto bkg: floatingBackgroundNames)
    {
      floatingHists.push_back(util::GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + bkg));
    }

    fixedSum.reset(static_cast<PlotUtils::MnvH1D*>(floatingHists.front()->Clone()));
    fixedSum->Reset();
    for(const auto& fixed: fixedNames) fixedSum->Add(util::GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_Background_" + fixed));

    //Let signal float too
    try
    {
      if(!floatSignal) fixedSum->Add(util::GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_TruthSignal"));
      else floatingHists.push_back(util::GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_TruthSignal"));
    }
    catch(const std::runtime_error& e)
    {
      if(!floatSignal) fixedSum->Add(util::GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_SelectedMCEvents"));
      else floatingHists.push_back(util::GetIngredient<PlotUtils::MnvH1D>(mcDir, sidebandName + "_SelectedMCEvents"));
    }
  }

  //Construct a Sideband in a specific, non-CV, systematic universe using the CV sideband as the source of histograms.
  //This makes sure the sideband histograms' modifications are written to the output file with the CV.
  Sideband::Sideband(const Sideband& cvSideband, const std::string& errorBandName, const int whichUniv): data(cvSideband.data), fixedSum(static_cast<TH1D*>(dynamic_cast<PlotUtils::MnvH1D*>(cvSideband.fixedSum.get())->GetVertErrorBand(errorBandName)->GetHist(whichUniv)->Clone()))
  {
    for(const auto cvHist: cvSideband.floatingHists)
    {
      floatingHists.push_back(dynamic_cast<PlotUtils::MnvH1D*>(cvHist)->GetVertErrorBand(errorBandName)->GetHist(whichUniv));
    }
  }

  //Make the compiler happy.
  Sideband::Sideband(const Sideband& parent): data(parent.data), fixedSum(static_cast<TH1D*>(parent.fixedSum->Clone())), floatingHists(parent.floatingHists)
  {
  }
}
