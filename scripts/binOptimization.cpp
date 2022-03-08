//File: binOptimization.cpp
//Brief: ROOT macro to optimize the bins of a migration matrix so that its diagonals all have a minimum fraction of their rows.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//MAT includes
#include "PlotUtils/MnvH2D.h"

//ROOT includes
#include "TFile.h"
#include "TCanvas.h"

//c++ includes
#include <iostream>

TH2* rebin(const TH2& source, const std::vector<double>& newBins)
{
  auto result = new TH2D(source.GetName(), source.GetTitle(), newBins.size()-1, newBins.data(), newBins.size()-1, newBins.data());
  constexpr double epsilon = 5e-3; //TODO: Can I get by without this?  I'm just trying to shove the integral into the previous bin.

  for(int xBin = 0; xBin < newBins.size(); ++xBin)
  {
    const int lowXBin = source.GetXaxis()->FindBin(newBins[xBin]),
              highXBin = source.GetXaxis()->FindBin(newBins[xBin + 1] - epsilon);
    for(int yBin = 0; yBin < newBins.size(); ++yBin)
    {
      const int lowYBin = source.GetYaxis()->FindBin(newBins[yBin]),
                highYBin = source.GetYaxis()->FindBin(newBins[yBin + 1] - epsilon);
      result->SetBinContent(result->GetBin(xBin+1, yBin+1), source.Integral(lowXBin, highXBin, lowYBin, highYBin));
    }
  }

  return result;
}

int binOptimization()
{
  constexpr double minFractionOfRow = 0.6; //Minimum fraction of the row that must be on the diagonal

  std::unique_ptr<TFile> file(TFile::Open("multiNeutronMC_merged.root"));
  auto hist = file->Get<PlotUtils::MnvH2D>("Tracker_MuonPTSignal_Migration");

  std::vector<double> newBinEdges(1, hist->GetXaxis()->GetBinLowEdge(0));

  for(int nextBin = 1; nextBin < hist->GetXaxis()->GetNbins(); ++nextBin) //Bin 0 is the underflow bin which has no lower edge anyway
  {
    const int firstBin = nextBin;
    double fractionOnDiag = hist->GetBinContent(hist->GetBin(nextBin, nextBin)) /  hist->Integral(0, hist->GetXaxis()->GetNbins()+1, firstBin, nextBin);
    while(fractionOnDiag < minFractionOfRow && nextBin <= hist->GetXaxis()->GetNbins())
    {
      std::cout << "Fraction of bin " << newBinEdges.size() << " on diagonal is " << fractionOnDiag << ", so keep adding more bins.\n";
      ++nextBin;
      const double sumRow = hist->Integral(0, hist->GetXaxis()->GetNbins()+1, firstBin, nextBin);
      fractionOnDiag = hist->Integral(firstBin, nextBin, firstBin, nextBin) / sumRow;
    }

    std::cout << "Stopped bin " << newBinEdges.size() << " when fraction on diagonal is " << fractionOnDiag << "\n";
    newBinEdges.push_back(hist->GetXaxis()->GetBinLowEdge(nextBin));
  }

  std::cout << "New bin edges: [";
  for(const auto edge: newBinEdges) std::cout << edge << ", "; //TODO: remove comma from last entry
  std::cout << "]\n";

  std::unique_ptr<TFile> outFile(TFile::Open("rebinned.root", "RECREATE"));
  auto rebinned = rebin(*hist, newBinEdges);
  outFile->Write();

  return 0;
}
