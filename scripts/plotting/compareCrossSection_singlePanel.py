#!/usr/bin/python

import ROOT
from ROOT import PlotUtils
import ctypes

ROOT.TH1.AddDirectory(False)

ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetEndErrorSize(4)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetPadBottomMargin(0.12)
ROOT.gStyle.SetPadLeftMargin(0.13)
ROOT.gStyle.SetPadRightMargin(0.02)
ROOT.gStyle.SetPadTopMargin(0.07) #0.12) #With title
ROOT.gStyle.SetTitleSize(0.056, "xyz") #0.055
ROOT.gStyle.SetTitleOffset(0.9, "y")
ROOT.gStyle.SetTitleOffset(1.1, "x")
ROOT.gStyle.SetLabelSize(0.06, "xyz")
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
  hist.GetYaxis().SetTitle("#frac{d#sigma}{dp_{T #mu}} [cm^{2} #times c / GeV / nucleon]")
  hist.GetXaxis().SetTitle("Muon Transverse Momentum [GeV/c]") #"p_{T #mu} [GeV/c]")

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
SuSACrossSection.SetTitle("SuSA 2p2h")
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
  #model.SetMarkerStyle(markerStyle)
  #model.SetMarkerColor(model.GetLineColor())
  #model.SetMarkerSize(1)
  markerStyle += 1

#Configure a 2-panel TCanvas with the main histogram on the top and the ratio on the bottom
rightMargin = 0.02
leftMargin = 0.13 #0.085
bottomFraction = 0.2
marginBetweenPads = 0.0938 #0.117175
labelSize = 0.15

can = ROOT.TCanvas("crossSection")
can.SetRightMargin(0)
can.SetLeftMargin(0)
top = ROOT.TPad("Top", "Top", 0, bottomFraction, 1, 1)
top.SetRightMargin(rightMargin)
top.SetLeftMargin(leftMargin)
top.SetTopMargin(0.085) #Tuned by hand to match GENIE cross section plotting script
bottom = ROOT.TPad("Ratio", "Ratio", 0, 0, 1, bottomFraction + marginBetweenPads)
bottom.SetRightMargin(rightMargin)
bottom.SetLeftMargin(leftMargin)
top.Draw()
bottom.Draw()

#Draw histograms
top.cd()
#legend = ROOT.TLegend(0.6, 0.4, 0.98, 0.88) #Poster size, but I want text size to match best size for GENIE comparison
legend = ROOT.TLegend(0.6, 0.55, 0.98, 0.88)

yMax = max([model.GetMaximum() for model in otherModels])
for model in otherModels:
  model.SetMaximum(yMax * 1.1)
  model.GetYaxis().SetTitleOffset(0.93) #0.75
  model.Draw("HIST SAME")
  #model.Draw("P HIST SAME")
  legend.AddEntry(model, "", "lf")

dataHist = dataCrossSection.GetCVHistoWithError()
dataHist.Draw("SAME E1")
legend.AddEntry(dataHist, "", "lpe")

dataStatBars = dataCrossSection.GetCVHistoWithStatError()
dataStatBars.Draw("SAME E1")

legend.Draw()

title = ROOT.TLatex(0.17, 0.95, "#bar{#nu}_{#mu} + CH -> #mu^{+} + Nn + X at N > 1 and E_{avail} < 100 MeV")
title.SetTextFont(43)
title.SetTextSize(22)
title.SetNDC()
#title.Draw()

#Ratio
bottom.cd()
bottom.SetTopMargin(0)
bottom.SetBottomMargin(0.375) #0.3) #Tuned by hand to match GENIE plotting script

ROOT.gStyle.SetEndErrorSize(4) #MnvPlotter undoes this somehow :(
denom = mnvTuneCrossSection.GetCVHistoWithStatError()

#Divide by just the CV of MnvTunev1 and leave the errors on the data alone
nBins = denom.GetXaxis().GetNbins()
for whichBin in range(0, nBins+1):
  denom.SetBinError(whichBin, 0)

dataRatio = dataCrossSection.GetCVHistoWithError()
dataRatio.Divide(dataRatio, denom)
dataRatio.SetMinimum(0)
dataRatio.GetYaxis().SetTitle("MnvTunev1 Ratio")
dataRatio.SetMaximum(1.35)

#Special axis configuration for 2-panel plot
dataRatio.GetYaxis().SetLabelSize(labelSize)
dataRatio.GetYaxis().SetTitleSize(0.1)
dataRatio.GetYaxis().SetTitleOffset(0.35)
dataRatio.GetYaxis().SetNdivisions(505) #5 minor divisions between 5 major divisions.  I'm trying to match a specific paper here.

dataRatio.GetXaxis().SetTitleSize(0.145)
dataRatio.GetXaxis().SetTitleOffset(1.08)
dataRatio.GetXaxis().SetLabelSize(0.16) #Tuned to match GENIE comparison
dataRatio.GetXaxis().SetLabelOffset(0.015) #Tuned to match GENIE comparison

dataRatio.Draw("E1")

ratios = [] #modelRatio 2 lines later is a temporary object.  Python will delete it, but ROOT will keep a pointer for THistPainter to use when Print() is called.  Putting it in a list prevent python from deleting it.
for model in otherModels:
  modelRatio = model.GetCVHistoWithStatError()
  ratios.append(modelRatio)
  modelRatio.Divide(modelRatio, denom)
  modelRatio.Draw("HIST SAME")
  #modelRatio.Draw("P HIST SAME")

#Draw markers on each model's lines so they can be distinguished without relying on colors
#TODO: symbols appear on top of the legend even though lines don't.  They also appear on top of the data.
#for ratio in ratios:
#  ratio.Draw("P HIST SAME")

dataRatioStatBars = dataCrossSection.GetCVHistoWithStatError()
dataRatioStatBars.Divide(dataRatioStatBars, denom)
dataRatioStatBars.Draw("SAME E1")

can.Print("crossSectionComp.png")
can.Print("crossSectionComp.pdf")

#Check whether any bins are < 0
dataRatio.SetMaximum(0.005)
dataRatio.SetMinimum(-0.005)
#title.Draw()
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
plotter.legend_offset_x = -0.3
plotter.legend_offset_y = -0.07
plotter.axis_title_offset_x = 1.1
plotter.axis_title_offset_y = 1.15
plotter.axis_title_size_x = 0.045
plotter.axis_title_size_y = 0.045
plotter.axis_title_font_x = 42 #Turn off default bold text
plotter.axis_title_font_y = 42
plotter.axis_title_font_z = 42
plotter.extra_top_margin = 0.07
plotter.extra_right_margin = 0.02
plotter.extra_left_margin = 0.13
plotter.extra_bottom_margin = 0.12

can.Clear()
can.SetTopMargin(0.07)
can.SetRightMargin(0.02)
can.SetLeftMargin(0.13)
can.SetBottomMargin(0.12)

plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, "", True, "", False, "L")
#gDirectory->Print()

#preliminary.Draw()
#title.Draw()
can.Print("uncertaintySummary.png")
can.Print("uncertaintySummary.pdf")

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
