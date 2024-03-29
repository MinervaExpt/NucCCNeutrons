//File: edepsWithRatioFromLEPaper.cpp
//Brief: Draws data and MC histograms on the same canvas with a ratio of Data/MC
//       on a canvas below.  Accepts any number of additional MC predictions and
//       and draws their ratios too.  This is my approximation to figure 5 of https://arxiv.org/pdf/1901.04892.pdf
//Author: Andrew Olivier aolivier@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvColors.h"
#include "PlotUtils/MnvPlotter.h"

//ROOT includes
#include "TFile.h"
#include "TKey.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TLegend.h"
#include "TPaveText.h"

//c++ includes
#include <iostream>
#include <string>
#include <regex>

//I hate global variables, but it's after 10PM...
const int lineSize = 3;
const double maxMC = 2; //Maximum across all plots I want to compare
const double minRatio = 0.5, maxRatio = 1.9;

PlotUtils::MnvH1D expandEntriesToMatchBinning(const PlotUtils::MnvH1D& templateHist, const PlotUtils::MnvH1D& nEntries)
{
  PlotUtils::MnvH1D expanded(templateHist);
  expanded.Reset("ICEM");

  const int nBins = expanded.GetXaxis()->GetNbins()+1; //+1 for the overflow bin

  //Set the CV to be the same number of entries in each bin
  for(int whichBin = 0; whichBin < nBins; ++whichBin) expanded.SetBinContent(whichBin, nEntries.GetBinContent(1)); //There's only 1 bin in nEntries, plus under/overflow

  //Do the same for systematics...
  for(const auto& bandName: nEntries.GetVertErrorBandNames()) //For each error band
  {
    const auto nEntriesBand = nEntries.GetVertErrorBand(bandName);
    const auto expandedBand = expanded.GetVertErrorBand(bandName);

    //TODO: Remove this kludge when I rerun NeutronDetectionStudy with afterAllFiles() fixed!
    for(int whichBin = 0; whichBin < nBins; ++whichBin) expandedBand->SetBinContent(whichBin, nEntries.GetBinContent(1));

    for(size_t whichUniv = 0; whichUniv < nEntriesBand->GetNHists(); ++whichUniv) //For each universe
    {
      const auto expandedHist = expandedBand->GetHist(whichUniv);
      for(int whichBin = 0; whichBin < nBins; ++whichBin) expandedHist->SetBinContent(whichBin, nEntriesBand->GetHist(whichUniv)->GetBinContent(1));
    }
  }

  return expanded;
}

THStack select(TFile& file, const std::regex& match)
{
  const auto rawEntries = file.Get<PlotUtils::MnvH1D>("Tracker_Neutron_Detection_NMCEntries");
  THStack found;

  for(auto key: *file.GetListOfKeys())
  {
    if(std::regex_match(key->GetName(), match))
    {
      auto hist = dynamic_cast<PlotUtils::MnvH1D*>(static_cast<TKey*>(key)->ReadObj());
      if(hist)
      {
        const auto nEntries = expandEntriesToMatchBinning(*hist, *rawEntries); //TODO: I could do this only once for the whole file, but then I'd have to choose a template histogram.  It's a script, so it shouldn't run for long enough to matter anyway, right?
        hist->Divide(hist, &nEntries);
        found.Add(static_cast<PlotUtils::MnvH1D*>(hist->Clone())); //static_cast<TH1D*>(hist->GetCVHistoWithError().Clone()));
      }
    }
  }

  return found;
}

TFile* giveMeFileOrGiveMeDeath(const std::string& fileName)
{
  auto file = TFile::Open(fileName.c_str());
  if(!file) throw std::runtime_error("Failed to open a file named " + fileName);
  return file;
}

void applyColors(TList& hists, const std::vector<int>& colors)
{
  for(int whichHist = 0; whichHist < hists.GetEntries(); ++whichHist)
  {
    auto& hist = dynamic_cast<TH1&>(*hists.At(whichHist));
    hist.SetLineColor(colors.at(whichHist));
    hist.SetLineWidth(lineSize);
    /*hist.SetMarkerStyle(20+whichHist);
    hist.SetMarkerColor(hist.GetLineColor());
    hist.SetMarkerSize(1);*/
    //hist.SetFillColor(colors.at(whichHist)); //Use only without nostack (Yes, that's a double negative)
  }
}

template <class ...ARGS>
int edepsWithRatioFromLEPaper(const std::string& dataFileName, const std::string& mcFileName, const ARGS&... otherMCFileNames)
{
  TH1::AddDirectory(false);

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0); //I'll draw it myself
  gStyle->SetTitleSize(0.08, "pad");

  auto dataFile = giveMeFileOrGiveMeDeath(dataFileName),
       mcFile   = giveMeFileOrGiveMeDeath(mcFileName);
  //std::vector<TFile*> otherMCFiles = {giveMeFileOrGiveMeDeath(otherMCFileName)...};

  const std::string var = "EDeps", anaName = "Tracker_Neutron_Detection",
                    dataName = anaName + "_Data" + var;
  const std::regex find(anaName + R"(__(.*))" + var);

  auto mcStack = select(*mcFile, find);
  auto dataHistRaw = dynamic_cast<PlotUtils::MnvH1D*>(dataFile->Get(dataName.c_str()));
  if(!dataHistRaw)
  {
    std::cerr << "Failed to find a histogram named " << dataName << "\n";
    return 1;
  }

  auto dataHist = dataHistRaw->GetCVHistoWithStatError();
  const auto rawEntries = dataFile->Get<PlotUtils::MnvH1D>((anaName + "_NDataEntries").c_str());
  const auto nEntries = expandEntriesToMatchBinning(dataHist, *rawEntries);
  dataHist.Divide(&dataHist, &nEntries);

  std::vector<TFile*> otherMCFiles = {giveMeFileOrGiveMeDeath(otherMCFileNames)...};
  std::vector<THStack> otherMCStacks;
  for(const auto& file: otherMCFiles)
  {
    auto stack = select(*file, find);
    stack.SetName(file->GetName());
    otherMCStacks.push_back(stack);
  }

  //Set histogram styles
  applyColors(*mcStack.GetHists(), MnvColors::GetColors(MnvColors::kOkabeItoDarkPalette));

  //Set up a Canvas split in 2.  The bottom canvas really overlaps the top by margin
  //to avoid drawing the x axis twice.
  TCanvas overall(("Data/MC for " + var).c_str());

  //Turns out that when you create a TPad while there's a TCanvas,
  //the canvas automatically becomes the parent of that TPad.
  const double bottomFraction = 0.2, margin = 0.078; //margin was tuned by hand
  TPad top("Overlay", "Overlay", 0, bottomFraction, 1, 1),
       bottom("Ratio", "Ratio", 0, 0, 1, bottomFraction + margin);
  //Thou shalt Draw() new TPads lest they be blank!
  top.Draw();
  bottom.Draw();

  const double labelSize = 0.15;

  //Data with stacked MC
  top.cd();

  auto mcTotal = static_cast<PlotUtils::MnvH1D*>(mcStack.GetStack()->Last())->GetCVHistoWithError();
  mcTotal.SetTitle("MnvTunev1");

  mcTotal.GetYaxis()->SetTitle("candidates / event"); //dataHist->GetYaxis()->GetTitle()); //TODO: I had the axes backwards in the original plo
  mcTotal.GetYaxis()->SetLabelSize(labelSize * (bottomFraction + margin));
  mcTotal.GetYaxis()->SetTitleSize(0.06); //TODO: How to guess what these are?
  mcTotal.GetYaxis()->SetTitleOffset(0.6);

  mcTotal.SetLineColor(kRed);
  mcTotal.SetFillColorAlpha(kPink + 1, 0.4);
  mcTotal.SetMaximum(maxMC);
  mcTotal.SetMarkerStyle(0);
  mcTotal.Draw("E2"); //Draw the error bars

  mcStack.Draw("HISTnostackSAME");
  mcStack.Draw("PnostackSAME"); //TODO: Prevent duplicate legend entries
  auto axes = mcStack.GetHistogram(); //N.B.: GetHistogram() would have returned nullptr had I called it before Draw()!

  dataHist.SetLineColor(1);
  dataHist.SetLineWidth(lineSize);
  dataHist.SetMarkerStyle(20); //Resizeable closed circle
  dataHist.SetMarkerColor(1);
  dataHist.SetMarkerSize(0.7);
  dataHist.SetTitle("Data");
  dataHist.Draw("X0SAME");

  auto topLegend = new TLegend(0.5, 0.4, 0.9, 0.9); //top.BuildLegend(0.5, 0.4, 0.9, 0.9);
  topLegend->AddEntry(&mcTotal, mcTotal.GetTitle(), "lf");
  for(auto hist: *mcStack.GetHists()) topLegend->AddEntry(hist, hist->GetTitle(), "lf");
  topLegend->AddEntry(&dataHist, dataHist.GetTitle(), "lpe");
  topLegend->Draw();

  //Drawing the thing that I don't want in the legend AFTER
  //building the legend.  What a dirty hack!
  //TODO: Not necessary with manual legend that I'm now using
  auto lineOnly = static_cast<TH1*>(mcTotal.Clone());
  lineOnly->SetFillStyle(0);
  lineOnly->Draw("HISTSAME"); //Draw the line

  //Data/MC ratio panel
  bottom.cd();
  bottom.SetTopMargin(0);
  bottom.SetBottomMargin(0.3);

  auto bottomLegend = new TLegend(0.12, 0.6, 0.42, 0.95);
  bottomLegend->SetBorderSize(0);

  auto ratio = static_cast<TH1D*>(dataHist.Clone());
  auto mcRatio = static_cast<PlotUtils::MnvH1D*>(mcStack.GetStack()->Last()->Clone());
  mcRatio->SetMarkerStyle(0);
  auto mcRatioWithErrors = mcRatio->GetCVHistoWithError();

  //I want a ratio histogram for the data that has only the data stat. errors.
  //I can't let the MC systematics get into the ratio because I'm not subtracting backgrounds (which would put systematics on the data too).
  //I can't let the MC statistical uncertainty get into the ratio because it too is already in the MC error envelope.
  auto mcDenom = mcRatio->GetCVHistoWithStatError();
  const int nDenomBins = mcDenom.GetXaxis()->GetNbins()+2; //1 extra for underflow and 1 extra for overflow
  for(int whichBin = 0; whichBin < nDenomBins; ++whichBin) mcDenom.SetBinError(whichBin, 0);
  ratio->Divide(ratio, &mcDenom);

  ratio->SetTitle("data");
  ratio->SetLineWidth(lineSize);
  ratio->SetTitleSize(0);

  ratio->GetYaxis()->SetTitle("Data / MC");
  ratio->GetYaxis()->SetLabelSize(labelSize);
  ratio->GetYaxis()->SetTitleSize(0.16);
  ratio->GetYaxis()->SetTitleOffset(0.2);
  ratio->GetYaxis()->SetNdivisions(505); //5 minor divisions between 5 major divisions

  ratio->GetXaxis()->SetTitleSize(0.16);
  ratio->GetXaxis()->SetTitleOffset(0.9);
  ratio->GetXaxis()->SetTitle("energy deposits [MeV]"); //dataHist->GetXaxis()->GetTitle()); //TODO: I had the axes backwards in the original plot
  ratio->GetXaxis()->SetLabelSize(labelSize);

  ratio->SetMinimum(minRatio);
  ratio->SetMaximum(maxRatio);
  ratio->Draw("E0X0");

  //Draw other models to compare to
  int whichLineStyle = 1;
  const std::string baseFileName = mcFileName.substr(0, mcFileName.find(".root"));
  for(auto& otherModel: otherMCStacks)
  {
    auto modelRatio = static_cast</*PlotUtils::MnvH1D**/TH1D*>(otherModel.GetStack()->Last()->Clone());

    std::string legendName = otherModel.GetName();
    //TODO: Next line isn't working.
    std::cout << "legendName is " << legendName << ".  baseFileName is " << baseFileName << "\n";
    legendName = legendName.substr(legendName.find(baseFileName) + baseFileName.length() + 1, std::string::npos); //+1 for the "_"
    legendName = legendName.substr(0, legendName.find(".root"));
    
    //Replace _s with spaces
    for(size_t foundUnderscore = legendName.find("_"); foundUnderscore != std::string::npos; foundUnderscore = legendName.find("_"))
    {
      legendName[foundUnderscore] = ' ';
    }

    modelRatio->SetTitle(legendName.c_str());
    modelRatio->SetLineStyle(whichLineStyle++);
    modelRatio->SetLineColor(kBlack);
    modelRatio->SetLineWidth(lineSize);

    modelRatio->Divide(modelRatio, &mcDenom); //TODO: I updated this to respect the MC's error envelope, but I haven't tested it with alternative models yet!

    modelRatio->SetMinimum(minRatio);
    modelRatio->SetMaximum(maxRatio);
    modelRatio->Draw("HIST SAME");
    bottomLegend->AddEntry(modelRatio);
  }

  //auto bottomLegend = bottom.BuildLegend(0.1, 0.6, 0.4, 0.95);
  bottomLegend->Draw();

  //Now fill mcRatio with 1 for bin content and fractional error
  for(int whichBin = 0; whichBin <= mcRatioWithErrors.GetXaxis()->GetNbins(); ++whichBin)
  {
    mcRatioWithErrors.SetBinError(whichBin, mcRatioWithErrors.GetBinError(whichBin)/mcRatioWithErrors.GetBinContent(whichBin));
    mcRatioWithErrors.SetBinContent(whichBin, 1);
  }

  mcRatioWithErrors.SetLineColor(kRed);
  mcRatioWithErrors.SetLineWidth(lineSize);
  mcRatioWithErrors.SetFillColorAlpha(kPink + 1, 0.4);
  mcRatioWithErrors.Draw("E2SAME");

  //Draw a flat line through the center of the MC
  auto straightLine = static_cast<TH1*>(mcRatioWithErrors.Clone());
  straightLine->SetFillStyle(0);
  straightLine->Draw("HISTSAME");
  //TODO: Do uncertainty propagation correctly.  Looks like I'm assuming data and MC are uncorrelated for now which is roughly true.

  //Title for the whole plot
  top.cd();
  /*TPaveText title(0.3, 0.91, 0.7, 1.0, "nbNDC"); //no border
  title.SetFillStyle(0);
  title.SetLineColor(0);
  title.AddText("Tracker"); //TODO: Get this from the file name?
  title.Draw();*/

  //MINERvA Preliminary
  TPaveText prelim(0.12, 0.78, 0.47, 0.85, "nbNDC"); //no border
  prelim.SetFillStyle(0);
  prelim.SetLineColor(0);
  prelim.SetTextColor(kBlue);
  //prelim.AddText("MINERvA Work in Progress"); //Preliminary");
  //prelim.AddText("Stat. Errors Only");
  std::string q3Tag;
  if(baseFileName.find("low") != std::string::npos)
  {
    prelim.AddText("q_{3} < 0.4 GeV/c");
    q3Tag = "lowq3";
  }
  else
  {
    prelim.AddText("0.4 GeV/c < q_{3} < 0.8 GeV/c");
    q3Tag = "highq3";
  }
  prelim.Draw();

  overall.Print((var + "_" + q3Tag + "_DataMCRatio.png").c_str()); //TODO: Include file name here
  overall.Print((var + "_" + q3Tag + "_DataMCRatio.pdf").c_str());

  //Plot uncertainty summary for sum on a new canvas
  TCanvas uncSummary("uncertaintySummary");
  PlotUtils::MnvPlotter plotter;
  plotter.ApplyStyle(PlotUtils::kCCQENuStyle);
  plotter.axis_maximum = 0.5;
  plotter.DrawErrorSummary(static_cast<PlotUtils::MnvH1D*>(mcStack.GetStack()->Last()->Clone()));
  uncSummary.Print("edepsUncertaintySummary.png");
  uncSummary.Print("edepsUncertaintySummary.pdf");

  return 0;
}
