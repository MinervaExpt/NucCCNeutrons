#!/usr/bin/python
#Usage: compareReco.py [multiNeutron_modelName.root ...]

import ROOT
from ROOT import PlotUtils
import sys

ROOT.gStyle.SetOptStat(0)
ROOT.TH1.AddDirectory(False)

"""Removes the low recoil fit as QE tail enhancement systematic universe from an MnvH1D"""
def removeLowRecoilQEErrorBand(hist):
  bandName = "Low_Recoil_2p2h_Tune"
  oldErrorBand = hist.PopVertErrorBand(bandName)
  if oldErrorBand:
    universes = oldErrorBand.GetHists()
    universes.erase(universes.begin() + 2)
    hist.AddVertErrorBand(bandName, universes)

def formatHist(hist):
  hist.SetLineWidth(3)
  #hist.GetYaxis().SetTitle("#frac{d#sigma}{dp_{T #mu}} [cm^{2} * c / GeV / nucleon]")
  hist.GetXaxis().SetTitle("Reconstructed p_{T #mu} [GeV/c]")

dataFile = ROOT.TFile.Open("crossSection_constrained.root")
dataDistribution = dataFile.Get("backgroundSubtracted")
dataDistribution.SetTitle("Data")
dataDistribution.SetLineColor(ROOT.kBlack)
dataPOT = 9.2950465e+20 #dataFile.Get("POTUsed").GetVal() #TODO: Write out POT in ExtractCrossSection?

otherModels = []
#mcColors = ROOT.PlotUtils.MnvColors.GetColors(MnvColors.kOkabeItoDarkPalette)
nextColor = 2 #0
for fileName in sys.argv[1:]:
  otherFile = ROOT.TFile.Open(fileName, "READ")
  otherPOT = 4.7994378e+21 #otherFile.Get("POTUsed").GetVal() #TODO: Write out POT in ExtractCrossSection?
  otherDistribution = otherFile.Get("backgroundSubtracted")
  print "Setting title for file " + fileName + " to " + fileName[fileName.rfind("_")+1:fileName.find(".root")]
  otherDistribution.SetTitle(fileName[fileName.rfind("_")+1:fileName.find(".root")])
  otherDistribution.SetLineColor(nextColor) #mcColors[nextColor])
  otherDistribution.Scale(dataPOT / otherPOT)
  otherModels.append(otherDistribution)

  nextColor = nextColor + 1

#mnvTuneFile = TFile.Open("crossSection_MnvTunev1.root")
#SuSAFile = TFile.Open("crossSection_SuSA.root")
#valenciaFile = TFile.Open("crossSection_Valencia.root")
#
#dataDistribution = dataFile.Get("backgroundSubtracted")
#dataDistribution.SetTitle("Data")
#dataDistribution.SetLineColor(kBlack)
#
#mnvTuneCrossSection = mnvTuneFile.Get("backgroundSubtracted")
#mnvTuneCrossSection.SetTitle("MnvTunev1")
#mnvTuneCrossSection.SetLineColor(kRed)
#
#SuSACrossSection = SuSAFile.Get("backgroundSubtracted")
#SuSACrossSection.SetTitle("SuSA")
#SuSACrossSection.SetLineColor(kBlue)
#
#ValenciaCrossSection = valenciaFile.Get("backgroundSubtracted")
#ValenciaCrossSection.SetTitle("Valencia 2p2h")
#ValenciaCrossSection.SetLineColor(kGreen+2)
#
#otherModels = [mnvTuneCrossSection, SuSACrossSection, ValenciaCrossSection]

removeLowRecoilQEErrorBand(dataDistribution)
formatHist(dataDistribution)

for model in otherModels:
  removeLowRecoilQEErrorBand(model)
  formatHist(model)

#Draw histograms
can = ROOT.TCanvas("backgroundSubtracted")

yMax = max([model.GetMaximum() for model in otherModels])
for model in otherModels:
  model.SetMaximum(yMax * 1.1)
  model.Draw("HIST SAME")

dataHist = dataDistribution.GetCVHistoWithError()
dataHist.Draw("SAME")

can.BuildLegend(0.6, 0.6, 0.9, 0.9)
otherModels[0].SetTitle("Background Subtracted")
can.Print("backgroundSubtractedComp.png")

#Draw uncertainty summary on the cross section extracted from data
plotter = PlotUtils.MnvPlotter()
plotter.ApplyStyle(PlotUtils.kCCQENuStyle)

#plotter.error_color_map["NeutronInelasticExclusives"] = kBlue
plotter.error_color_map["UnifiedCrossTalk"] = ROOT.kBlue

plotter.error_summary_group_map["response"].push_back("response_proton") #= ["response_proton", "reponse_em", "respone_other"]
plotter.error_summary_group_map["response"].push_back("response_em")
plotter.error_summary_group_map["response"].push_back("response_meson")
plotter.error_summary_group_map["response"].push_back("response_other")
plotter.error_color_map["response"] = ROOT.kMagenta+2

#plotter.error_summary_group_map["more GEANT"] = ["GEANT_Neutron", "GEANT_Pion", "GEANT_Proton"]
plotter.error_summary_group_map["GEANT"].push_back("NeutronInelasticExclusives")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Neutron")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Pion")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Proton")

plotter.axis_maximum = 0.5
plotter.DrawErrorSummary(dataDistribution)
can.Print("uncertaintySummary.png")

plotter.DrawErrorSummary(dataDistribution, "TR", True, True, 0.00001, False, "Cross Section Models")
can.Print("uncertaintySummary_models.png")

plotter.DrawErrorSummary(dataDistribution, "TR", True, True, 0.00001, False, "FSI Models")
can.Print("uncertaintySummary_FSI.png")

#Ratio
#denom = mnvTuneCrossSection.Clone()
#
#dataDistribution.Divide(dataDistribution, denom)
#dataRatio = dataDistribution.GetCVHistoWithError()
#dataRatio.SetMinimum(0)
#dataRatio.GetYaxis().SetTitle("Ratio to MnvTunev1")
#dataRatio.Draw()
#
#for model in otherModels:
#  model.Divide(model, denom)
#  model.Draw("HIST SAME")
#
#can.BuildLegend(0.6, 0.6, 0.9, 0.9)
#can.Print("backgroundSubtractedRatio.png")

#Check whether any bins are < 0
#dataRatio.SetMaximum(0.005)
#dataRatio.SetMinimum(-0.005)
#dataRatio.Draw()
#can.Print("ZoomedDataRatio.png")
