//File: ExtractCrossSection.cpp
//Brief: Given data and MC files from analyses/studies/CrossSection.h, extract a 1D differential cross section.
//       Subtracts backgrounds, performs unfolding, applies efficiency x acceptance correction, and 
//       divides by flux and number of nucleons.  Writes a .root file with the cross section histogram.
//
//Usage: ExtractCrossSection <unfolding iterations> <data.root> <mc.root>
//
//Author: Andrew Olivier aolivier@ur.rochester.edu

//util includes
#include "util/GetIngredient.h"

//UnfoldUtils includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "MinervaUnfold/MnvUnfold.h"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvH2D.h"
#include "PlotUtils/MnvPlotter.h"
#pragma GCC diagnostic pop

//ROOT includes
#include "TH1D.h"
#include "TFile.h"
#include "TKey.h"
#include "TParameter.h"
#include "TCanvas.h"

//Cintex is only needed for older ROOT versions like the GPVMs.
////Let CMake decide whether it's needed.
#ifndef NCINTEX
#include "Cintex/Cintex.h"
#endif

//c++ includes
#include <iostream>
#include <exception>
#include <algorithm>
#include <numeric>

namespace
{
  std::map<std::string, std::vector<std::string>> errorGroups = {{"CCQE Model and RPA", {"GENIE_CCQEPauliSupViaKF", "GENIE_NormCCQE", "GENIE_VecFFCCQEshape", "GENIE_MaCCQEshape", "RPA_LowQ2", "RPA_HighQ2"}},
                                                                 {"FSI", {"GENIE_FrAbs_N", "GENIE_FrCEx_N", "GENIE_FrElas_N", "GENIE_FrInel_N", "GENIE_MFP_N", "GENIE_FrAbs_pi", "GENIE_FrCEx_pi", "GENIE_FrElas_pi", "GENIE_FrPiProd_pi", "GENIE_MFP_pi"}},
                                                                 //{"RES Model", {"GENIE_MaRES", "GENIE_MvRES"}},
                                                                 {"GEANT", {"GEANT_Neutron", "GEANT_Proton", "GEANT_Pion"}},
                                                                 {"Flux", {"Flux"}},
                                                                 //{"RPA_LowQ2", {"RPA_LowQ2"}},
                                                                 {"2p2h Tune", {"Low_Recoil_2p2h_Tune"}}};
}

//Convince the STL to talk to TIter so I can use std::find_if()
namespace std
{
  template <>
  struct iterator_traits<TIter>
  {
    using value_type = TObject;
    using pointer = TObject*;
    using reference = TObject&;
    using iterator_category = forward_iterator_tag;
  };
}

//Plot a step in cross section extraction.
void Plot(PlotUtils::MnvH1D& hist, const std::string& stepName, const std::string& prefix)
{
  TCanvas can(stepName.c_str());
  hist.GetCVHistoWithError().Clone()->Draw();
  can.Print((prefix + "_" + stepName + ".png").c_str());

  //Uncertainty summary
  //Put unused error bands into the "Other" category
  const auto allBandNames = hist.GetVertErrorBandNames();
  auto& other = ::errorGroups["Other"];
  for(const auto& name: allBandNames)
  {
    if(std::find_if(::errorGroups.begin(), errorGroups.end(),
                    [&name](const auto& group)
                    { return std::find(group.second.begin(), group.second.end(), name) != group.second.end(); })
       == ::errorGroups.end())
    {
      other.push_back(name);
    }
  }

  //Set up a MnvPlotter
  PlotUtils::MnvPlotter plotter;
  plotter.ApplyStyle(PlotUtils::kCCQENuStyle);
  plotter.error_summary_group_map["GEANT"] = {"GEANT_Pion", "GEANT_Proton"};
  plotter.error_color_map["NeutronInelasticExclusives"] = kBlue;
  //plotter.error_summary_group_map = ::errorGroups;
  plotter.axis_maximum = 0.4;

  plotter.DrawErrorSummary(&hist);
  can.Print((prefix + "_" + stepName + "_uncertaintySummary.png").c_str());

  plotter.DrawErrorSummary(&hist, "TR", true, true, 1e-5, false, "Other");
  can.Print((prefix + "_" + stepName + "_otherUncertainties.png").c_str());
}

//Unfolding function from Aaron Bercelle
//TODO: Trim it down a little?  Remove that static?
PlotUtils::MnvH1D* UnfoldHist( PlotUtils::MnvH1D* h_folded, PlotUtils::MnvH2D* h_migration, int num_iter )
{
  /*static*/ MinervaUnfold::MnvUnfold unfold;
  PlotUtils::MnvH1D* h_unfolded = nullptr;

  //bool bUnfolded = false;

  TMatrixD dummyCovMatrix;
  const RooUnfold::Algorithm unfoldingAlg = (num_iter < 0)?RooUnfold::kInvert:RooUnfold::kBayes; //If number of iterations was given on the command line, use MINERvA standard d'Agostini unfolding.  Otherwise, try to invert the migration matrix.
  std::cout << "Using unfolding algorithm " << unfoldingAlg << "\n";
  if(!unfold.UnfoldHisto( h_unfolded, dummyCovMatrix, h_migration, h_folded, unfoldingAlg, num_iter, true, false ))
    return nullptr;

  /////////////////////////////////////////////////////////////////////////////////////////  
  //No idea if this is still needed
  //Probably.  This gets your stat unfolding covariance matrix
  TMatrixD unfoldingCovMatrixOrig; 
  int correctNbins;
  int matrixRows;  
  TH1D* hUnfoldedDummy  = new TH1D(h_unfolded->GetCVHistoWithStatError());
  TH1D* hRecoDummy      = new TH1D(h_migration->ProjectionX()->GetCVHistoWithStatError());
  TH1D* hTruthDummy     = new TH1D(h_migration->ProjectionY()->GetCVHistoWithStatError());
  TH1D* hBGSubDataDummy = new TH1D(h_folded->GetCVHistoWithStatError());
  TH2D* hMigrationDummy = new TH2D(h_migration->GetCVHistoWithStatError());
  unfold.UnfoldHisto(hUnfoldedDummy, unfoldingCovMatrixOrig, hMigrationDummy, hRecoDummy, hTruthDummy, hBGSubDataDummy, unfoldingAlg, num_iter);//Stupid RooUnfold.  This is dummy, we don't need iterations

  correctNbins=hUnfoldedDummy->fN;
  matrixRows=unfoldingCovMatrixOrig.GetNrows();
  if(correctNbins!=matrixRows){
    std::cout << "****************************************************************************" << std::endl;
    std::cout << "*  Fixing unfolding matrix size because of RooUnfold bug. From " << matrixRows << " to " << correctNbins << std::endl;
    std::cout << "****************************************************************************" << std::endl;
    // It looks like this, since the extra last two bins don't have any content
    unfoldingCovMatrixOrig.ResizeTo(correctNbins, correctNbins);
  }

  for(int i=0; i<unfoldingCovMatrixOrig.GetNrows(); ++i) unfoldingCovMatrixOrig(i,i)=0;
  delete hUnfoldedDummy;
  delete hMigrationDummy;
  delete hRecoDummy;
  delete hTruthDummy;
  delete hBGSubDataDummy;
  h_unfolded->PushCovMatrix("unfoldingCov",unfoldingCovMatrixOrig);

  /////////////////////////////////////////////////////////////////////////////////////////  
  return h_unfolded;
}

//Inflate a histogram with only 1 bin to a histogram that matches the binning of another histogram but has the same value in each bin.
//Useful for storing a constant per systematic universe.
//TODO: This doesn't do anything about propagating statistical uncertainties
PlotUtils::MnvH1D* expandBinning(const PlotUtils::MnvH1D* toExpand, const PlotUtils::MnvH1D* toMatch)
{
  assert(toExpand->GetXaxis()->GetNbins() == 1 && "expandBinning() only works with histograms that have exactly 1 bin");
  auto result = toMatch->Clone((std::string(toExpand->GetName()) + "_expanded").c_str());
  result->Clear("ICEM"); //Keep only the binning

  //Put the same content of toExpand's single bin in each bin of the CV...
  const int nBins = result->GetXaxis()->GetNbins();
  for(int whichBin = 0; whichBin <= nBins; ++whichBin) result->SetBinContent(whichBin, toExpand->GetBinContent(1));

  //...and in each bin of each universe
  const auto bandNames = result->GetVertErrorBandNames();
  for(const auto& bandName: bandNames)
  {
    auto band = result->GetVertErrorBand(bandName);
    for(size_t whichUniv = 0; whichUniv < band->GetNHists(); ++whichUniv)
    {
      auto hist = band->GetHist(whichUniv);
      const double expandedContent = toExpand->GetVertErrorBand(bandName)->GetHist(whichUniv)->GetBinContent(1);
      for(int whichBin = 0; whichBin <= nBins; ++whichBin) hist->SetBinContent(whichBin, expandedContent);
    }
  }

  return result;
}

//The final step of cross section extraction: normalize by flux, bin width, POT, and number of targets
PlotUtils::MnvH1D* normalize(PlotUtils::MnvH1D* efficiencyCorrected, PlotUtils::MnvH1D* fluxIntegral, const PlotUtils::MnvH1D* nNucleons, const double POT)
{
  efficiencyCorrected->Divide(efficiencyCorrected, fluxIntegral);
  efficiencyCorrected->Divide(efficiencyCorrected, nNucleons);

  efficiencyCorrected->Scale(1./POT);
  efficiencyCorrected->Scale(1.e4); //Flux histogram is in m^-2, but convention is to report cm^2
  efficiencyCorrected->Scale(1., "width");

  return efficiencyCorrected;
}

//Print a list of bins that are less than 0 for every universe
bool checkForNegativeBins(const PlotUtils::MnvH1D& hist)
{
  int nNegativeBins = 0;
  constexpr double threshold = 0; //Make it easy to check for nearly 0 bins if I choose

  //First, check the CV
  for(int whichBin = 0; whichBin < hist.GetXaxis()->GetNbins(); ++whichBin)
  {
    const double content = hist.GetBinContent(whichBin);
    if(content < threshold)
    {
      ++nNegativeBins;
      std::cout << "Bin " << whichBin << " in the CV is < 0: " << content << "\n";
    }
  }

  //Now, check each universe too.  These can get messed up by weird sideband fits.
  for(const auto& bandName: hist.GetVertErrorBandNames())
  {
    const auto& band = hist.GetVertErrorBand(bandName);
    for(unsigned int whichUniv = 0; whichUniv < band->GetNHists(); ++whichUniv)
    {
      const auto& univ = band->GetHist(whichUniv);
      for(int whichBin = 0; whichBin < hist.GetXaxis()->GetNbins(); ++whichBin)
      {
        const double content = univ->GetBinContent(whichBin);
        if(content < threshold)
        {
          ++nNegativeBins;
          std::cout << "Bin " << whichBin << " in error band " << bandName << " universe " << whichUniv << " is < 0: " << content << "\n";
        }
      }
    }
  }

  return nNegativeBins;
}

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(kFALSE); //Needed so that MnvH1D gets to clean up its own MnvLatErrorBands (which are TH1Ds).

  if(argc < 3 || argc > 4)
  {
    std::cerr << "Expected 2 or 3 arguments, but I got " << argc-1 << ".\n"
              << "USAGE: ExtractCrossSection <unfolding iterations> <data.root> <mc.root>\n"
              << "       ExtractCrossSection <data.root> <mc.root>\n";
    return 1;
  }

  int firstFileArg = 2,
      nIterations = -1;
  if(argc == 4) nIterations = std::stoi(argv[1]);
  else firstFileArg = 1;

  std::unique_ptr<TFile> dataFile(TFile::Open(argv[firstFileArg], "READ"));
  if(!dataFile)
  {
    std::cerr << "Failed to open data file " << argv[firstFileArg] << ".\n";
    return 2;
  }

  std::unique_ptr<TFile> mcFile(TFile::Open(argv[firstFileArg+1], "READ"));
  if(!mcFile)
  {
    std::cerr << "Failed to open MC file " << argv[firstFileArg+1] << ".\n";
    return 3;
  }

  std::vector<std::string> crossSectionPrefixes;
  for(auto key: *dataFile->GetListOfKeys())
  {
    const std::string keyName = key->GetName();
    const size_t endOfPrefix = keyName.find("_Signal");
    if(endOfPrefix != std::string::npos) crossSectionPrefixes.push_back(keyName.substr(0, endOfPrefix));
  }

  const double mcPOT = util::GetIngredient<TParameter<double>>(*mcFile, "POTUsed")->GetVal(),
               dataPOT = util::GetIngredient<TParameter<double>>(*dataFile, "POTUsed")->GetVal();

  for(const auto& prefix: crossSectionPrefixes)
  {
    try
    {
      auto flux = util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, "reweightedflux_integrated", prefix);
      auto folded = util::GetIngredient<PlotUtils::MnvH1D>(*dataFile, "Signal", prefix);
      Plot(*folded, "data", prefix);
      auto migration = util::GetIngredient<PlotUtils::MnvH2D>(*mcFile, "Migration", prefix);
      auto effNum = util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, "EfficiencyNumerator", prefix);
      auto effDenom = util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, "EfficiencyDenominator", prefix);
      auto simEventRate = effDenom->Clone(); //Make a copy for later

      const auto fiducialFound = std::find_if(mcFile->GetListOfKeys()->begin(), mcFile->GetListOfKeys()->end(),
                                              [&prefix](const auto key)
                                              {
                                                const std::string keyName = key->GetName();
                                                const size_t fiducialEnd = keyName.find("_FiducialNucleons");
                                                return (fiducialEnd != std::string::npos) && (prefix.find(keyName.substr(0, fiducialEnd)) != std::string::npos);
                                              });
      if(fiducialFound == mcFile->GetListOfKeys()->end()) throw std::runtime_error("Failed to find a number of nucleons that matches prefix " + prefix);

      const auto nNucleons = expandBinning(util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, (*fiducialFound)->GetName()), effNum); //Dan: Use the same truth fiducial volume for all extractions.  The acceptance correction corrects data back to this fiducial even if the reco fiducial cut is different.

      //Look for backgrounds with <prefix>_<analysis>_Background_<name>
      std::vector<PlotUtils::MnvH1D*> backgrounds;
      for(auto key: *mcFile->GetListOfKeys())
      {
        if(std::string(key->GetName()).find(prefix + "_Background_") != std::string::npos)
        {
          backgrounds.push_back(util::GetIngredient<PlotUtils::MnvH1D>(*mcFile, key->GetName()));
        }
      }

      //There are no error bands in the data, but I need somewhere to put error bands on the results I derive from it.
      folded->AddMissingErrorBandsAndFillWithCV(*migration);

      //Basing my unfolding procedure for a differential cross section on Alex's MINERvA 101 talk at https://minerva-docdb.fnal.gov/cgi-bin/private/RetrieveFile?docid=27438&filename=whatsACrossSection.pdf&version=1

      auto toSubtract = std::accumulate(std::next(backgrounds.begin()), backgrounds.end(), (*backgrounds.begin())->Clone(),
                                        [](auto sum, const auto hist)
                                        {
                                          sum->Add(hist);
                                          return sum;
                                        });
      Plot(*toSubtract, "BackgroundSum", prefix);

      auto bkgSubtracted = std::accumulate(backgrounds.begin(), backgrounds.end(), folded->Clone(),
                                           [mcPOT, dataPOT](auto sum, const auto hist)
                                           {
                                             std::cout << "Subtracting " << hist->GetName() << " scaled by " << -dataPOT/mcPOT << " from " << sum->GetName() << "\n";
                                             sum->Add(hist, -dataPOT/mcPOT);
                                             return sum;
                                           });
      Plot(*bkgSubtracted, "backgroundSubtracted", prefix);

      std::unique_ptr<TFile> outFile(TFile::Open((prefix + "_crossSection.root").c_str(), "CREATE"));
      if(!outFile)
      {
        std::cerr << "Could not create a file called " << prefix + "_crossSection.root" << ".  Does it already exist?\n";
        return 5;
      }

      std::cout << "Checking background-subtracted histogram for negative bins...\n";
      if(!checkForNegativeBins(*bkgSubtracted)) std::cout << "...and found none!\n";

      bkgSubtracted->Write("backgroundSubtracted");

      //d'Aogstini unfolding
      auto unfolded = UnfoldHist(bkgSubtracted, migration, nIterations);
      if(!unfolded) throw std::runtime_error(std::string("Failed to unfold ") + folded->GetName() + " using " + migration->GetName());
      Plot(*unfolded, "unfolded", prefix);
      unfolded->Clone()->Write("unfolded");

      effNum->Divide(effNum, effDenom); //Only the 2 parameter version of MnvH1D::Divide()
                                        //handles systematics correctly.
      Plot(*effNum, "efficiency", prefix);

      unfolded->Divide(unfolded, effNum);
      Plot(*unfolded, "efficiencyCorrected", prefix);

      auto crossSection = normalize(unfolded, flux, nNucleons, dataPOT);
      Plot(*crossSection, "crossSection", prefix);
      crossSection->Clone()->Write("crossSection");

      //Write a "simulated cross section" to compare to the data I just extracted.
      //If this analysis passed its closure test, this should be the same cross section as
      //what GENIEXSecExtract would produce.
      normalize(simEventRate, flux, nNucleons, mcPOT);
      
      Plot(*simEventRate, "simulatedCrossSection", prefix);
      simEventRate->Write("simulatedCrossSection");
    }
    catch(const std::runtime_error& e)
    {
      std::cerr << "Failed to extra a cross section for prefix " << prefix << ": " << e.what() << "\n";
      return 4;
    }
  }

  return 0;
}
