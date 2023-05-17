#!/usr/bin/python

import ROOT
from ROOT import PlotUtils
import ctypes

ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetEndErrorSize(4)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetPadBottomMargin(0.15)
ROOT.gStyle.SetPadLeftMargin(0.15)
ROOT.gStyle.SetPadTopMargin(0.12)
ROOT.gStyle.SetTitleSize(0.055, "xyz")
ROOT.gStyle.SetTitleOffset(1.15, "xyz")
ROOT.gStyle.SetLabelSize(0.05, "xyz")
ROOT.gROOT.ForceStyle()

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

markerStyle = 20 #Make lines different to be more colorblind-friendly
for model in otherModels:
  removeLowRecoilQEErrorBand(model)
  formatHist(model)
  model.SetMarkerStyle(markerStyle)
  model.SetMarkerColor(model.GetLineColor())
  model.SetMarkerSize(1)
  markerStyle += 1

#Draw histograms
can = ROOT.TCanvas("crossSection")

legend = ROOT.TLegend(0.6, 0.58, 0.9, 0.88)

yMax = max([model.GetMaximum() for model in otherModels])
for model in otherModels:
  model.SetMaximum(yMax * 1.1)
  model.Draw("HIST SAME")
  model.Draw("P HIST SAME")
  legend.AddEntry(model)

dataHist = dataCrossSection.GetCVHistoWithError()
dataHist.Draw("SAME E1")
legend.AddEntry(dataHist)

dataStatBars = dataCrossSection.GetCVHistoWithStatError()
dataStatBars.Draw("SAME E1")

legend.Draw()

title = ROOT.TLatex(0.17, 0.95, "#bar{#nu}_{#mu} + CH -> #mu^{+} + Nn + X at N > 1 and E_{avail} < 100 MeV")
title.SetTextFont(43)
title.SetTextSize(22)
title.SetNDC()
title.Draw()

#preliminary = ROOT.TText(0.2, 0.83, "MINERvA Preliminary")
#preliminary.SetTextFont(43)
#preliminary.SetTextSize(22)
#preliminary.SetNDC()
#preliminary.SetTextColor(ROOT.kBlue)
#preliminary.Draw()

can.Print("crossSectionComp.png")
can.Print("crossSectionComp.eps")

#Ratio
ROOT.gStyle.SetEndErrorSize(4) #MnvPlotter undoes this somehow :(
denom = mnvTuneCrossSection.GetCVHistoWithStatError()

#Divide by just the CV of MnvTunev1 and leave the errors on the data alone
nBins = denom.GetXaxis().GetNbins()
for whichBin in range(0, nBins+1):
  denom.SetBinError(whichBin, 0)

legend = ROOT.TLegend(0.4, 0.73, 0.9, 0.88)
legend.SetNColumns(2)

dataRatio = dataCrossSection.GetCVHistoWithError()
dataRatio.Divide(dataRatio, denom)
dataRatio.SetMinimum(0)
dataRatio.GetYaxis().SetTitle("Ratio to MnvTunev1")
dataRatio.SetMaximum(1.35)
dataRatio.Draw("E1")
legend.AddEntry(dataRatio)

ratios = [] #modelRatio 2 lines later is a temporary object.  Python will delete it, but ROOT will keep a pointer for THistPainter to use when Print() is called.  Putting it in a list prevent python from deleting it.
for model in otherModels:
  modelRatio = model.GetCVHistoWithStatError()
  ratios.append(modelRatio)
  modelRatio.Divide(modelRatio, denom)
  modelRatio.Draw("HIST SAME")
  modelRatio.Draw("P HIST SAME")
  legend.AddEntry(modelRatio)

#Draw markers on each model's lines so they can be distinguished without relying on colors
#TODO: symbols appear on top of the legend even though lines don't.  They also appear on top of the data.
#for ratio in ratios:
#  ratio.Draw("P HIST SAME")

dataRatioStatBars = dataCrossSection.GetCVHistoWithStatError()
dataRatioStatBars.Divide(dataRatioStatBars, denom)
dataRatioStatBars.Draw("SAME E1")

legend.Draw()
title.Draw()
#preliminary.DrawText(0.2, 0.23, "MINERvA Prelminary")
can.Print("crossSectionRatio.png")
can.Print("crossSectionRatio.eps")

#Check whether any bins are < 0
dataRatio.SetMaximum(0.005)
dataRatio.SetMinimum(-0.005)
title.Draw()
dataRatio.Draw()
#preliminary.DrawText(0.2, 0.23, "MINERvA Prelminary")
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

plotter.axis_maximum = 1
#plotter.legend_n_columns = 2
plotter.legend_offset_x = -0.2
plotter.legend_offset_y = -0.07
plotter.axis_title_offset_x = 1.1
plotter.axis_title_offset_y = 1.1
plotter.axis_title_size_x = 0.06
plotter.axis_title_size_y = 0.06

plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, "", True, "", False, "L")
#preliminary.Draw()
title.Draw()
can.Print("uncertaintySummary.png")
can.Print("uncertaintySummary.eps")

plotter.legend_n_columns = 2
plotter.legend_offset_x = 0.12
for group in plotter.error_summary_group_map:
  plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, group.first, True, "", False, "L")
  title.DrawText(0.2, 0.93, group.first)
  can.Print("uncertaintySummary_" + group.first.replace(" ", "_") + ".png")

#Chi2 table in Markdown
with open("chi2Table.md", "w") as textFile:
  textFile.write("| Model Name | Chi2 / NDF |\n")
  textFile.write("| ---------- | ---------- |\n")
  for model in otherModels:
    ndf = ctypes.c_int(0)
    textFile.write("| " + model.GetTitle() + " | " + str(round(plotter.Chi2DataMC(dataCrossSection, model, ndf, 1.0, True, False, True), 2)) + " / " + str(ndf.value) + " |\n")
