#!/usr/bin/python

import ROOT
from ROOT import PlotUtils

ROOT.gStyle.SetOptStat(0)

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
dataHist.Draw("SAME")

can.BuildLegend(0.6, 0.6, 0.9, 0.9)
can.Print("crossSectionComp.png")

#Draw uncertainty summary on the cross section extracted from data
plotter = PlotUtils.MnvPlotter()
plotter.ApplyStyle(PlotUtils.kCCQENuStyle)

#plotter.error_color_map["NeutronInelasticExclusives"] = ROOT.kBlue
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
plotter.DrawErrorSummary(dataCrossSection)
can.Print("uncertaintySummary.png")

plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, "Cross Section Models")
can.Print("uncertaintySummary_models.png")

plotter.DrawErrorSummary(dataCrossSection, "TR", True, True, 0.00001, False, "FSI Models")
can.Print("uncertaintySummary_FSI.png")

#TODO: Ratio
denom = mnvTuneCrossSection.Clone()

dataCrossSection.Divide(dataCrossSection, denom)
dataRatio = dataCrossSection.GetCVHistoWithError()
dataRatio.SetMinimum(0)
dataRatio.GetYaxis().SetTitle("Ratio to MnvTunev1")
dataRatio.Draw()

for model in otherModels:
  model.Divide(model, denom)
  model.Draw("HIST SAME")

can.BuildLegend(0.6, 0.6, 0.9, 0.9)
can.Print("crossSectionRatio.png")

#Check whether any bins are < 0
dataRatio.SetMaximum(0.005)
dataRatio.SetMinimum(-0.005)
dataRatio.Draw()
can.Print("ZoomedDataRatio.png")
