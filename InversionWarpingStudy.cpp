//File: InversionWarpingStudy.cpp
//Brief: Test how sensitive RooUnfoldInverse is to statistical fluctuations.  Does some of the same things as TransWarpExtractor but where number of iterations is irrelevant.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#define USAGE "USAGE: InversionWarpingStudy <num_stat_universes> <CV.root> <otherModel.root>"

//UnfoldUtils includes
#include "MinervaUnfold/MnvUnfold.h"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvH2D.h"
#include "PlotUtils/MnvPlotter.h"

//ROOT includes
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"

//c++ includes
#include <iostream>
#include <random>
#include <chrono>

//Procedure from Aaron Bercellie long long ago in a galaxy far far away...
PlotUtils::MnvH1D* unfoldHist(MinervaUnfold::MnvUnfold& unfolder, const PlotUtils::MnvH1D& folded, const PlotUtils::MnvH2D& migration) //Pointers.  Why does it always have to be pointers?
{
  PlotUtils::MnvH1D* unfolded = nullptr; 

  TMatrixD dummyCovMatrix;
  const RooUnfold::Algorithm unfoldingAlg = RooUnfold::kInvert;
  if(!unfolder.UnfoldHisto( unfolded, dummyCovMatrix, &migration, &folded, unfoldingAlg, 3, true, false ))
    return nullptr;

  //No idea if this is still needed
  //Probably.  This gets your stat unfolding covariance matrix
  TMatrixD unfoldingCovMatrixOrig;
  int correctNbins;
  int matrixRows;
  TH1D* hUnfoldedDummy  = new TH1D(unfolded->GetCVHistoWithStatError());
  TH1D* hRecoDummy      = new TH1D(migration.ProjectionX()->GetCVHistoWithStatError());
  TH1D* hTruthDummy     = new TH1D(migration.ProjectionY()->GetCVHistoWithStatError());
  TH1D* hBGSubDataDummy = new TH1D(folded.GetCVHistoWithStatError());
  TH2D* hMigrationDummy = new TH2D(migration.GetCVHistoWithStatError());
  unfolder.UnfoldHisto(hUnfoldedDummy, unfoldingCovMatrixOrig, hMigrationDummy, hRecoDummy, hTruthDummy, hBGSubDataDummy, unfoldingAlg, 1);//Stupid RooUnfold.  This is dummy, we don't need iterations

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
  unfolded->PushCovMatrix("unfoldingCov",unfoldingCovMatrixOrig);

  return unfolded;
}

int main(const int argc, const char** argv)
{
  TH1::AddDirectory(kFALSE);

  if(argc != 4)
  {
    std::cerr << "Expected exactly 3 arguments but got " << argc-1 << "\n" << USAGE << "\n";
    return 1;
  }

  constexpr int chi2sForLimits = 20;
  const int nStatUnivs = std::stoi(argv[1]);
  const std::string cvFileName = argv[2];
  const std::string warpFileName = argv[3];  

  auto cvFile = TFile::Open(cvFileName.c_str()); //TODO: Check for null
  auto warpFile = TFile::Open(warpFileName.c_str()); //TODO: Check for null

  const auto warpedTruthDist = warpFile->Get<PlotUtils::MnvH1D>("Tracker_MuonPTSignal_EfficiencyNumerator");
  const auto warpedRecoDist = warpFile->Get<PlotUtils::MnvH1D>("Tracker_MuonPTSignal_SelectedMCEvents");
  const auto cvMigrationMatrix = cvFile->Get<PlotUtils::MnvH2D>("Tracker_MuonPTSignal_Migration");

  std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
  MinervaUnfold::MnvUnfold unfolder;

  std::vector<TH1D> fluctuationRecoDists(nStatUnivs, *warpedRecoDist);

  //Add statistical fluctuations to the reconstructed distribution
  const int nBins = warpedTruthDist->GetXaxis()->GetNbins();
  int ndf = nBins-1; //TODO: Is this strictly true?  Remember that MnvPlotter::Chi2DataMC() resets it anyway.
  for(int whichBin = 0; whichBin < nBins+2; ++whichBin)
  {
    std::poisson_distribution<long int> dist(warpedRecoDist->GetBinContent(whichBin));
    for(int whichStatUniv = 0; whichStatUniv < nStatUnivs; ++whichStatUniv)
    {
      const double newContent = dist(gen);
      fluctuationRecoDists[whichStatUniv].SetBinContent(whichBin, newContent);
      fluctuationRecoDists[whichStatUniv].SetBinError(whichBin, std::sqrt(newContent));
    }
  }

  //Prepare warpedTrthDist for chi2 calculations.  Following procedure from UnfoldUtils' TransWarpExtractor
  warpedTruthDist->ClearAllErrorBands();
  for(int whichBin = 0; whichBin < warpedTruthDist->fN; ++whichBin)
  {
    const double content = warpedTruthDist->GetBinContent(whichBin);
    warpedTruthDist->SetBinError(whichBin, std::sqrt(content));
  }

  //Compare fluctuated histograms after unfolding to warpedTruthDist

  auto outFile = TFile::Open(("InversionWarpingStudy_" + cvFileName.substr(0, cvFileName.find(".")) + "_warpedBy_" + warpFileName).c_str(), "CREATE"); //TODO: Check for null
  auto chi2Comparison = new TH1D("fluctuatedWarpsChi2s", (std::string("Unfolding #Chi^{2}s for Warp ") + warpFileName.substr(0, cvFileName.find(".")) + ";#Chi^{2} [NDF = " + std::to_string(ndf) + "];Stat. Universes").c_str(), 40, 0, 5*ndf); //Should implicitly be written to outFile

  PlotUtils::MnvPlotter plotter;
  
  //First, figure out appropriate axis limits by scanning the first few chi2s.
  double minChi2 = std::numeric_limits<double>::max(),
         maxChi2 = std::numeric_limits<double>::min();
  for(int whichStatUniv = 0; whichStatUniv < std::min(chi2sForLimits, nStatUnivs); ++whichStatUniv)
  {
    //TODO: This may be a performance problem because it inverts the histogram over and over again
    const std::unique_ptr<PlotUtils::MnvH1D> unfolded(unfoldHist(unfolder, fluctuationRecoDists[whichStatUniv], *cvMigrationMatrix));
    const double chi2 = plotter.Chi2DataMC(unfolded.get(), warpedTruthDist, ndf, 1, true, false, false); //Last optional argument is chi2 by bin which I'm not using
    minChi2 = std::min(minChi2, chi2);
    maxChi2 = std::max(maxChi2, chi2);
  }

  chi2Comparison->SetBins(40, 0.8*minChi2, 1.2*maxChi2);

  for(int whichStatUniv; whichStatUniv < nStatUnivs; ++whichStatUniv)
  {
    //TODO: This may be a performance problem because it inverts the histogram over and over again
    //std::cout << "Integral before unfolding: " << fluctuationRecoDists[whichStatUniv].Integral(0, nBins+2) << "\n";
    //fluctuationRecoDists[whichStatUniv].Print("all");
    const std::unique_ptr<PlotUtils::MnvH1D> unfolded(unfoldHist(unfolder, fluctuationRecoDists[whichStatUniv], *cvMigrationMatrix));
    chi2Comparison->Fill(plotter.Chi2DataMC(unfolded.get(), warpedTruthDist, ndf, 1, true, false, false)); //Last optional argument is chi2 by bin which I'm not using
    /*std::cout << "Integral after unfolding: " << unfolded->Integral(0, nBins+2) << "\n";
    unfolded->Print("all");*/

    /*warpedTruthDist->SetLineColor(kBlack);
    warpedTruthDist->Draw();

    unfolded->SetLineColor(kRed);
    unfolded->Draw("SAME");
    gPad->Print(("universe" + std::to_string(whichStatUniv) + ".png").c_str());*/
  }

  //TODO: Fluctuate the migration matrix too?  Sounds interesting and seems like something Kevin has asked about before.

  //Write results to a .root file for later perusal
  chi2Comparison->Write();
  outFile->Write();

  return 0;
}
