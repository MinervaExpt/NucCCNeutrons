//File: ExtractCrossSection.cpp
//Brief: Given data and MC files from analyses/studies/CrossSection.h, extract a 1D differential cross section.
//       Subtracts backgrounds, performs unfolding, applies efficiency x acceptance correction, and 
//       divides by flux and number of nucleons.  Writes a .root file with the cross section histogram.
//
//Usage: ExtractCrossSection <number of d'Agostini unfolding iterations> <data.root> <mc.root>
//
//Author: Andrew Olivier aolivier@ur.rochester.edu

//UnfoldUtils includes
#include "UnfoldUtils/MnvUnfold.h"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvH2D.h"

//ROOT includes
#include "TH1D.h"
#include "TFile.h"

//c++ includes
#include <iostream>

//Unfolding function from Aaron Bercelle
//TODO: Trim it down a little?  Remove that static?
MnvH1D* UnfoldHist( PlotUtils::MnvH1D* h_folded, PlotUtils::MnvH2D* h_migration, int num_iter )
{
  static MinervaUnfold::MnvUnfold unfold;
  MnvH1D* h_unfolded = nullptr;

  bool bUnfolded = false;

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
    cout << "****************************************************************************" << endl;
    cout << "*  Fixing unfolding matrix size because of RooUnfold bug. From " << matrixRows << " to " << correctNbins << endl;
    cout << "****************************************************************************" << endl;
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
  if(argc != 4)
  {
    std::cerr << "Expected 3 arguments, but I got " << argc-1 << ".\n"
              << "USAGE: ExtractCrossSection <number of d'Agostini unfolding iterations> <data.root> <mc.root>\n";
    return 1;
  }

  const int nIterations = std::stoi(argv[1]);
  auto dataFile = TFile::Open(argv[2]),
  if(!dataFile)
  {
    std::cerr << "Failed to open data file " << argv[2] << ".\n";
    return 2;
  }

  if(!mcFile)
  {
    std::cerr << "Failed to open MC file " << argv[3] << ".\n";
    return 3;
  }

  auto folded = dynamic_cast<PlotUtils::MnvH1D*>(dataFile->Get(""));
  auto migration = dynamic_cast<PlotUtils::MnvH2D*>(mcFile->Get(""));
  auto effNum = dynamic_cast<PlotUtils::MnvH1D*>(mcFile->Get(""));
  auto effDenom = dynamic_cast<PlotUtils::MnvH1D*>(mcFile->Get(""));

  //Basing my unfolding procedure for a differential cross section on Alex's MINERvA 101 talk at https://minerva-docdb.fnal.gov/cgi-bin/private/RetrieveFile?docid=27438&filename=whatsACrossSection.pdf&version=1

  //TODO: Background subtraction, possibly with sidebands
  auto bkgSubtracted = folded;

  //d'Aogstini unfolding
  auto unfolded = UnfoldHist(bkgSubtracted, migration, nIterations);
  if(!unfolded)
  {
    std::cerr << "Failed to unfold " << folded->GetName() << " using " << migration->GetName() << "\n";
    return 4;
  }

  //Efficiency and acceptance correction.
  auto eff = effNum->Divide(effNum, effDenom); //Only the 2 parameter version of MnvH1D::Divide()
                                               //handles systematics correctly.
  unfolded->Divide(unfolded, eff);

  //Divide by integrated flux

  //Divide by number of nucleons in the tracker

  //Normalize by bin width at the very end

  return 0;
}
