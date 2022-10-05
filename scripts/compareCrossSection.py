#!/usr/bin/python

import ROOT
from ROOT import PlotUtils

ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetEndErrorSize(4)
ROOT.gStyle.SetOptTitle(0)

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
  hist.GetYaxis().SetTitle("#frac{d#sigma}{dp_{T #mu}} [cm^{2} * c / GeV / nucleon]")
  hist.GetXaxis().SetTitle("p_{T #mu} [GeV/c]")

#TODO: Turn other models back on
dataFile = ROOT.TFile.Open("crossSection_constrained.root") #"crossSection_constrained.root")
mnvTuneFile = ROOT.TFile.Open("crossSection_MnvTunev1.root")
SuSAFile = ROOT.TFile.Open("crossSection_SuSA.root")
valenciaFile = ROOT.TFile.Open("crossSection_Valencia.root")

dataCrossSection = dataFile.Get("crossSection")
dataCrossSection.SetTitle("Data")
dataCrossSection.SetLineColor(ROOT.kBlack)

mnvTuneCrossSection = mnvTuneFile.Get("crossSection")
mnvTuneCrossSection.SetTitle("MnvTunev1")
mnvTuneCrossSection.SetLineColor(ROOT.kRed)

SuSACrossSection = SuSAFile.Get("crossSection")
SuSACrossSection.SetTitle("SuSA")
SuSACrossSection.SetLineColor(ROOT.kBlue)

ValenciaCrossSection = valenciaFile.Get("crossSection")
ValenciaCrossSection.SetTitle("Valencia 2p2h")
ValenciaCrossSection.SetLineColor(ROOT.kGreen+2)

otherModels = [mnvTuneCrossSection, SuSACrossSection, ValenciaCrossSection]

removeLowRecoilQEErrorBand(dataCrossSection)
formatHist(dataCrossSection)

for model in otherModels:
  removeLowRecoilQEErrorBand(model)
  formatHist(model)

#Draw histograms
can = ROOT.TCanvas("crossSection")

yMax = max([model.GetMaximum() for model in otherModels])
for model in otherModels:
  model.SetMaximum(yMax * 1.1)
  model.Draw("HIST SAME")

dataHist = dataCrossSection.GetCVHistoWithError()
dataHist.Draw("SAME E1")

can.BuildLegend(0.6, 0.6, 0.9, 0.9)

dataStatBars = dataCrossSection.GetCVHistoWithStatError()
dataStatBars.Draw("SAME E1")

title = ROOT.TLatex(0.2, 0.93, "#bar{#nu}_{#mu} + C^{12} -> #mu^{+} + n + n + X at E_{avail} < 100 MeV")
title.SetTextFont(43)
title.SetTextSize(22)
title.SetNDC()
title.Draw()

can.Print("crossSectionComp.png")

#Ratio
ROOT.gStyle.SetEndErrorSize(4) #MnvPlotter undoes this somehow :(
denom = mnvTuneCrossSection.GetCVHistoWithStatError()

#Divide by just the CV of MnvTunev1 and leave the errors on the data alone
nBins = denom.GetXaxis().GetNbins()
for whichBin in range(0, nBins+1):
  denom.SetBinError(whichBin, 0)

dataRatio = dataCrossSection.GetCVHistoWithError()
dataRatio.Divide(dataRatio, denom)
dataRatio.SetMinimum(0)
dataRatio.GetYaxis().SetTitle("Ratio to MnvTunev1")
dataRatio.Draw("E1")

dontDeleteUs = [] #modelRatio 2 lines later is a temporary object.  Python will delete it, but ROOT will keep a pointer for THistPainter to use when Print() is called.  Putting it in a list prevent python from deleting it.
for model in otherModels:
  modelRatio = model.GetCVHistoWithStatError()
  dontDeleteUs.append(modelRatio)
  modelRatio.Divide(modelRatio, denom)
  modelRatio.Draw("HIST SAME")

can.BuildLegend(0.6, 0.6, 0.9, 0.9)
dataRatioStatBars = dataCrossSection.GetCVHistoWithStatError()
dataRatioStatBars.Divide(dataRatioStatBars, denom)
dataRatioStatBars.Draw("SAME E1")

title.Draw()
can.Print("crossSectionRatio.png")

#Check whether any bins are < 0
dataRatio.SetMaximum(0.005)
dataRatio.SetMinimum(-0.005)
title.Draw()
dataRatio.Draw()
can.Print("ZoomedDataRatio.png")

#Draw uncertainty summary on the cross section extracted from data
plotter = PlotUtils.MnvPlotter()
plotter.ApplyStyle(PlotUtils.kCCQENuStyle)
plotter.axis_title_offset_x = 1.25
plotter.axis_title_size_x = 0.037
plotter.axis_title_offset_y = 1.25
plotter.axis_title_size_y = 0.04

plotter.error_color_map["Initial State Models"] = ROOT.TColor.GetColor("#b87f00")
plotter.error_color_map["Detector Response"]    = ROOT.TColor.GetColor("#4590ba")
plotter.error_color_map["FSI Models"]           = ROOT.TColor.GetColor("#007e5c")
plotter.error_color_map["Flux"]                 = ROOT.TColor.GetColor("#c0b635")
plotter.error_color_map["GEANT"]                = ROOT.TColor.GetColor("#005b8e")
plotter.error_color_map["Muon Reconstruction"]  = ROOT.TColor.GetColor("#aa4b00")
plotter.error_color_map["Others"]               = ROOT.kBlue

plotter.error_summary_group_map["Detector Response"].push_back("response_proton")
plotter.error_summary_group_map["Detector Response"].push_back("response_em")
plotter.error_summary_group_map["Detector Response"].push_back("response_meson")
plotter.error_summary_group_map["Detector Response"].push_back("response_other")
plotter.error_summary_group_map["Detector Response"].push_back("UnifiedCrossTalk")

plotter.error_summary_group_map["GEANT"].push_back("NeutronInelasticExclusives")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Neutron")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Pion")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Proton")

plotter.error_summary_group_map["Muon Reconstruction"].push_back("MuonAngleXResolution")
plotter.error_summary_group_map["Muon Reconstruction"].push_back("MuonAngleYResolution")

plotter.error_summary_group_map["Others"].push_back("Target_Mass_CH")

#Rename "Cross Section Models" to "Initial State Interactions" to try to satisfy Kevin
plotter.error_summary_group_map["Initial State Models"].push_back("Low_Recoil_2p2h_Tune")
plotter.error_summary_group_map.erase(plotter.error_summary_group_map.find("Low Recoil Fits"))
oldGroupNames = plotter.error_summary_group_map["Cross Section Models"]
plotter.error_summary_group_map["Initial State Models"].insert(plotter.error_summary_group_map["Initial State Models"].end(), oldGroupNames.begin(), oldGroupNames.end())
plotter.error_summary_group_map["Initial State Models"].push_back("SignalModel")
plotter.error_summary_group_map.erase(plotter.error_summary_group_map.find("Cross Section Models"))

plotter.axis_maximum = 0.5
plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, "", True, "", False, "L")
title.Draw()
can.Print("uncertaintySummary.png")

for group in plotter.error_summary_group_map:
  plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, group.first, True, "", False, "L")
  title.DrawText(0.2, 0.93, group.first)
  can.Print("uncertaintySummary_" + group.first.replace(" ", "_") + ".png")
