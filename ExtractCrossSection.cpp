//File: ExtractCrossSection.cpp
//Brief: Given data and MC files from analyses/studies/CrossSection.h, extract a 1D differential cross section.
//       Subtracts backgrounds, performs unfolding, applies efficiency x acceptance correction, and 
//       divides by flux and number of nucleons.  Writes a .root file with the cross section histogram.
//
//Usage: ExtractCrossSection <unfolding iterations> <data.root> <mc.root>
//
//Author: Andrew Olivier aolivier@ur.rochester.edu

//UnfoldUtils includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "UnfoldUtils/MnvUnfold.h"

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
  std::map<std::string, std::vector<std::string>> errorGroups = {{"CCQE Model", {"GENIE_CCQEPauliSupViaKF", "GENIE_NormCCQE", "GENIE_VecFFCCQEshape", "GENIE_MaCCQEshape"}},
                                                                 {"Nucleon FSI", {"GENIE_FrAbs_N", "GENIE_FrCEx_N", "GENIE_FrElas_N", "GENIE_FrInel_N", "GENIE_MFP_N"}},
                                                                 {"Pion FSI", {"GENIE_FrAbs_pi", "GENIE_FrCEx_pi", "GENIE_FrElas_pi", "GENIE_FrPiProd_pi", "GENIE_MFP_pi"}},
                                                                 {"GENIE_NormCCRES", {"GENIE_NormCCRES"}},
                                                                 {"Flux", {"Flux"}},
                                                                 {"RPA_LowQ2", {"RPA_LowQ2"}},
                                                                 {"2p2h", {"2p2h"}}};
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
  plotter.error_summary_group_map = ::errorGroups;

  plotter.DrawErrorSummary(&hist);
  can.Print((prefix + "_" + stepName + "_uncertaintySummary.png").c_str());
}

template <class TYPE>
TYPE* GetIngredient(TDirectoryFile& dir, const std::string& ingredient, const std::string& prefix)
{
  return GetIngredient<TYPE>(dir, prefix + "_" + ingredient);
}

//Unfolding function from Aaron Bercelle
//TODO: Trim it down a little?  Remove that static?
PlotUtils::MnvH1D* UnfoldHist( PlotUtils::MnvH1D* h_folded, PlotUtils::MnvH2D* h_migration, int num_iter )
{
  static MinervaUnfold::MnvUnfold unfold;
  PlotUtils::MnvH1D* h_unfolded = nullptr;

  //bool bUnfolded = false;

  TMatrixD dummyCovMatrix;
  if(!unfold.UnfoldHisto( h_unfolded, dummyCovMatrix, h_migration, h_folded, RooUnfold::kBayes, num_iter, true, false ))
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
  unfold.UnfoldHisto(hUnfoldedDummy, unfoldingCovMatrixOrig, hMigrationDummy, hRecoDummy, hTruthDummy, hBGSubDataDummy,RooUnfold::kBayes, num_iter);//Stupid RooUnfold.  This is dummy, we don't need iterations

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

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(kFALSE); //Needed so that MnvH1D gets to clean up its own MnvLatErrorBands (which are TH1Ds).

  if(argc != 4)
  {
    std::cerr << "Expected 3 arguments, but I got " << argc-1 << ".\n"
              << "USAGE: ExtractCrossSection <unfolding iterations> <data.root> <mc.root>\n";
    return 1;
  }

  const int nIterations = std::stoi(argv[1]);
  auto dataFile = TFile::Open(argv[2]);
  if(!dataFile)
  {
    std::cerr << "Failed to open data file " << argv[2] << ".\n";
    return 2;
  }

  auto mcFile = TFile::Open(argv[3]);
  if(!mcFile)
  {
    std::cerr << "Failed to open MC file " << argv[3] << ".\n";
    return 3;
  }

  std::vector<std::string> crossSectionPrefixes;
  for(auto key: *dataFile->GetListOfKeys())
  {
    const std::string keyName = key->GetName();
    const size_t endOfPrefix = keyName.find("_Signal");
    if(endOfPrefix != std::string::npos) crossSectionPrefixes.push_back(keyName.substr(0, endOfPrefix));
  }

  const double mcPOT = GetIngredient<TParameter<double>>(*mcFile, "POTUsed")->GetVal(),
               dataPOT = GetIngredient<TParameter<double>>(*mcFile, "POTUsed")->GetVal();

  for(const auto& prefix: crossSectionPrefixes)
  {
    try
    {
      auto flux = GetIngredient<PlotUtils::MnvH1D>(*mcFile, "reweightedflux_integrated", prefix);
      auto folded = GetIngredient<PlotUtils::MnvH1D>(*dataFile, "Signal", prefix);
      Plot(*folded, "data", prefix);
      auto migration = GetIngredient<PlotUtils::MnvH2D>(*mcFile, "Migration", prefix);
      auto effNum = GetIngredient<PlotUtils::MnvH1D>(*mcFile, "EfficiencyNumerator", prefix);
      auto effDenom = GetIngredient<PlotUtils::MnvH1D>(*mcFile, "EfficiencyDenominator", prefix);

      const auto fiducialFound = std::find_if(mcFile->GetListOfKeys()->begin(), mcFile->GetListOfKeys()->end(),
                                              [&prefix](const auto key)
                                              {
                                                const std::string keyName = key->GetName();
                                                const size_t fiducialEnd = keyName.find("_FiducialNucleons");
                                                return (fiducialEnd != std::string::npos) && (prefix.find(keyName.substr(0, fiducialEnd)) != std::string::npos);
                                              });
      if(fiducialFound == mcFile->GetListOfKeys()->end()) throw std::runtime_error("Failed to find a number of nucleons that matches prefix " + prefix);

      auto nNucleons = GetIngredient<TParameter<double>>(*mcFile, (*fiducialFound)->GetName());

      //Look for backgrounds with <prefix>_<analysis>_Background_<name>
      std::vector<PlotUtils::MnvH1D*> backgrounds;
      for(auto key: *mcFile->GetListOfKeys())
      {
        if(std::string(key->GetName()).find(prefix + "_Background_") != std::string::npos)
        {
          backgrounds.push_back(GetIngredient<PlotUtils::MnvH1D>(*mcFile, key->GetName()));
        }
      }

      //There are no error bands in the data, but I need somewhere to put error bands on the results I derive from it.
      folded->AddMissingErrorBandsAndFillWithCV(*migration);

      //Basing my unfolding procedure for a differential cross section on Alex's MINERvA 101 talk at https://minerva-docdb.fnal.gov/cgi-bin/private/RetrieveFile?docid=27438&filename=whatsACrossSection.pdf&version=1

      //TODO: How to tie in sideband constraints?
      //TODO: Scale backgrounds by ratio of data POT to MC POT?
      auto bkgSubtracted = std::accumulate(backgrounds.begin(), backgrounds.end(), folded,
                                           [mcPOT, dataPOT](auto sum, const auto hist)
                                           {
                                             sum->Add(hist, -dataPOT/mcPOT);
                                             return sum;
                                           });
      Plot(*bkgSubtracted, "backgroundSubtracted", prefix);

      //d'Aogstini unfolding
      auto unfolded = UnfoldHist(bkgSubtracted, migration, nIterations);
      if(!unfolded) throw std::runtime_error(std::string("Failed to unfold ") + folded->GetName() + " using " + migration->GetName());
      Plot(*unfolded, "unfolded", prefix);

      effNum->Divide(effNum, effDenom); //Only the 2 parameter version of MnvH1D::Divide()
                                        //handles systematics correctly.
      Plot(*effNum, "efficiency", prefix);

      unfolded->Divide(unfolded, effNum);
      Plot(*unfolded, "efficiencyCorrected", prefix);

      unfolded->Divide(unfolded, flux);
      unfolded->Scale(1./nNucleons->GetVal()/dataPOT);
      unfolded->Scale(1., "width");
      Plot(*unfolded, "crossSection", prefix);
    }
    catch(const std::runtime_error& e)
    {
      std::cerr << "Failed to extra a cross section for prefix " << prefix << ": " << e.what() << "\n";
      return 4;
    }
  }

  return 0;
}
