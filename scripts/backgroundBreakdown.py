#!/usr/bin/python

#USAGE: backgroundStack.py <dataFile.root> <mcFile.root>

from ROOT import *
from ROOT import PlotUtils

import sys

gROOT.SetBatch() #Don't render histograms to a window.  Also gets filled areas correct.

var = "MuonPT"
fiducialName = "Tracker"
ratioMin = 0.5
ratioMax = 1.5
maxY = 8e4 #TODO: Base this on the selection region.  Maybe the max of selection region and all sidebands if I'm fancy.

bottomFraction = 0.2
marginBetweenPads = 0.117175 #Tuned by hand
leftMargin = 0.085
rightMargin = 0.02
labelSize = 0.15
lineSize = 2

TH1.AddDirectory(False)
dataFile = TFile.Open(sys.argv[1])
mcFile = TFile.Open(sys.argv[2])
plotter = PlotUtils.MnvPlotter()

mcPOT = mcFile.Get("POTUsed").GetVal()
dataPOT = dataFile.Get("POTUsed").GetVal()

def drawStack(sidebandName, isSelected = False):
  #Deal with some inconsistencies in how I decided long ago to name my selection region and sidebands
  dataName = fiducialName + "_" + sidebandName + "_"
  if isSelected:
    dataName += "Signal"
  else:
    dataName += "Data"

  mcSignalName = fiducialName + "_" + sidebandName + "_"
  if isSelected:
    mcSignalName += "SelectedMCEvents"
  else:
    mcSignalName += "TruthSignal"

  #Organize the MC backgrounds into a stacked histogram.
  #Also keep a sum of backgrounds that has full systematics
  #information.
  mcStack = THStack()

  signalHist = mcFile.Get(mcSignalName)
  signalHist.SetTitle("Signal") #For the selection region, the title is a little too descriptive
  signalHist.Scale(dataPOT/mcPOT)

  mcSum = signalHist.Clone()
  for key in mcFile.GetListOfKeys():
    name = str(key.GetName())
    if name.find(fiducialName + "_" + sidebandName + "_" + "Background") > -1:
      print "Found a background named " + key.GetName()
      hist = key.ReadObj()
      hist.Scale(dataPOT/mcPOT)
      mcStack.Add(hist.GetCVHistoWithError().Clone())
      mcSum.Add(hist)
  mcStack.Add(signalHist.GetCVHistoWithError().Clone())
  mcStack.GetHists().Sort(True) #Sort histograms alphabetically by title
  
  #Apply a different color for each MC category
  mcColors = MnvColors.GetColors(MnvColors.kOkabeItoDarkPalette)
  nextColor = 0
  gStyle.SetHatchesLineWidth(6)
  for hist in mcStack.GetHists():
    hist.SetLineColor(mcColors[nextColor])
    hist.SetFillColor(mcColors[nextColor])
    hist.SetFillStyle(3550+2*nextColor)
    nextColor = nextColor + 1
  
  dataHist = dataFile.Get(dataName)
  dataWithStatErrors = dataHist.GetCVHistoWithError().Clone()
  dataHist.AddMissingErrorBandsAndFillWithCV(signalHist)
  
  #Create a TCanvas on which to draw plots and split it into 2 panels
  #TODO: TCanvas seems to have a different default size when I plot with pyROOT instead of interpretted ROOT,
  #      and default TPad children won't fill it in the horizontal direction!
  overall = TCanvas("Data/MC for " + var, "", 700, 500)
  overall.SetRightMargin(0)
  overall.SetLeftMargin(0)
  top = TPad("Overlay", "Overlay", 0, bottomFraction, 1, 1)
  top.SetRightMargin(rightMargin)
  top.SetLeftMargin(leftMargin)
  bottom = TPad("Ratio", "Ratio", 0, 0, 1, bottomFraction + marginBetweenPads)
  bottom.SetRightMargin(rightMargin)
  bottom.SetLeftMargin(leftMargin)
  #Thou shalt Draw() new TPads lest they be blank!
  top.Draw()
  bottom.Draw()
  
  top.cd()
  mcTotal = mcStack.GetStack().Last()
  mcTotal.SetMinimum(0)
  mcTotal.SetMaximum(maxY)
  mcTotal.Draw("E2") #Draw the error envelope only.  Not used here except to force the THStack to calculate its sum.
  
  mcStack.SetMinimum(1)
  mcStack.Draw("HIST")
  mcStack.GetHistogram().GetYaxis().SetTitleOffset(0.6)
  mcStack.GetHistogram().GetYaxis().SetTitle("entries")
  #mcStack.GetHistogram().GetYaxis().SetTitleSize(0.05) #Works, but still bold
  #mcStack.GetHistogram().GetYaxis().SetTitleFont(142) #Works, but still bold
  mcStack.GetHistogram().GetYaxis().SetTitleFont(42)
  mcStack.Draw("HIST")
  
  dataWithStatErrors.SetLineColor(kBlack)
  dataWithStatErrors.SetLineWidth(lineSize)
  dataWithStatErrors.SetMarkerStyle(20) #Resizeable closed circle
  dataWithStatErrors.SetMarkerColor(kBlack)
  dataWithStatErrors.SetMarkerSize(0.7)
  dataWithStatErrors.SetTitle("Data")
  dataWithStatErrors.Draw("SAME")
  
  legend = top.BuildLegend(0.68, 0.35, 1.-rightMargin, 0.91)
  
  #Data/MC ratio panel
  bottom.cd()
  bottom.SetTopMargin(0)
  bottom.SetBottomMargin(0.3)
  
  ratio = dataHist.Clone()
  mcTotalWithSys = mcSum
  ratio.Divide(ratio, mcTotalWithSys)
  
  #TODO: I need GetCVHistoWithError() from mcRatio, but THStack doesn't keep a MnvH1D.  I have to Add() the histograms myself.
  
  #Now fill mcRatio with 1 for bin content and fractional error
  mcRatio = mcTotalWithSys.GetTotalError(False, True, False) #The second "true" makes this fractional error
  for whichBin in range(1, mcRatio.GetXaxis().GetNbins()+1):
    mcRatio.SetBinError(whichBin, max(mcRatio.GetBinContent(whichBin), 1e-9))
    mcRatio.SetBinContent(whichBin, 1)
  
  ratio.SetTitle("")
  ratio.SetLineColor(kBlack)
  ratio.SetLineWidth(lineSize)
  ratio.SetTitleSize(0)
  
  ratio.GetYaxis().SetTitle("Data / MC")
  ratio.GetYaxis().SetLabelSize(labelSize)
  ratio.GetYaxis().SetTitleSize(0.16)
  ratio.GetYaxis().SetTitleOffset(0.25)
  ratio.GetYaxis().SetNdivisions(505) #5 minor divisions between 5 major divisions.  I'm trying to match a specific paper here.
  
  ratio.GetXaxis().SetTitleSize(0.16)
  ratio.GetXaxis().SetTitleOffset(0.9)
  ratio.GetXaxis().SetLabelSize(labelSize)
  
  ratio.SetMinimum(ratioMin)
  ratio.SetMaximum(ratioMax)
  ratio.GetXaxis().SetTitle("Reconstructed Muon Transverse Momentum [GeV/c]")
  ratio.Draw()
  
  #Error envelope for the MC
  mcRatio.SetLineColor(kRed)
  mcRatio.SetLineWidth(lineSize)
  mcRatio.SetFillColorAlpha(kPink + 1, 0.4)
  mcRatio.Draw("E2 SAME")
  
  #Draw a flat line at 1 for ratio of MC to itself
  straightLine = mcRatio.Clone()
  straightLine.SetFillStyle(0)
  straightLine.Draw("HIST SAME")
  
  #Title for the whole plot
  top.cd()
  title = TPaveText(0.3, 0.91, 0.7, 1.0, "nbNDC") #no border and use Normalized Device Coordinates to place it
  title.SetFillStyle(0)
  title.SetLineColor(kWhite)
  if isSelected:
    title.AddText("Selected") #fiducialName + " Selected")
  else:
    if sidebandName == "QELike":
      title.AddText("0-1 Neutrons")
    else:
      title.AddText(sidebandName) #fiducialName + " " + sidebandName)
  title.Draw()
  
  #plotter.WritePreliminary(0.4, 0.82, 7e-2, True)
  
  #Make a PNG file of this canvas
  overall.Print(fiducialName + var + sidebandName + "DataMCRatio.png")

  del overall #Make sure that the TPad deletes the THStack before the THStack deletes itself.  Otherwise, this often crashes with "virtual method called".

#Draw stack for selection region and then loop over sidebands
drawStack(var+"Signal", True)

for key in mcFile.GetListOfKeys():
  signalTagLocation = key.GetName().find("TruthSignal")
  if signalTagLocation > -1:
    print "Sideband name is " + key.GetName()[key.GetName().find(fiducialName)+len(fiducialName)+1 : signalTagLocation-1]
    drawStack(key.GetName()[key.GetName().find(fiducialName)+len(fiducialName)+1 : signalTagLocation-1])
