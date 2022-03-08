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

GENIEDir = "/media/anaTuples/multiNeutronPredictions/GENIEv3/"

dataFile = ROOT.TFile.Open("crossSection_constrained.root")
mnvTuneFile = ROOT.TFile.Open("crossSection_MnvTunev1.root")
genieHANievesFile = ROOT.TFile.Open(GENIEDir + "G18_10a_02_11a_set_0_.root")
genieHNNievesFile = ROOT.TFile.Open(GENIEDir + "G18_10b_02_11a_set_0_.root")
genieHAEmpiricalFile = ROOT.TFile.Open(GENIEDir + "G18_02a_02_11a_set_0_.root")
genieHNEmpiricalFile = ROOT.TFile.Open(GENIEDir + "G18_02b_02_11a_set_0_.root")

dataCrossSection = dataFile.Get("crossSection")
dataCrossSection.SetTitle("Data")
dataCrossSection.SetLineColor(ROOT.kBlack)

mnvTuneCrossSection = mnvTuneFile.Get("crossSection")
mnvTuneCrossSection.SetTitle("MnvTunev1")
mnvTuneCrossSection.SetLineColor(ROOT.kRed)

genieHANieves = genieHANievesFile.Get("pt")
genieHANieves.SetTitle("GENIE v3 hA Nieves QE+2p2h")
genieHANieves.SetLineColor(ROOT.kBlue)
genieHANieves.Scale(1., "width")

genieHNNieves = genieHNNievesFile.Get("pt")
genieHNNieves.SetTitle("GENIE v3 hN Nieves QE+2p2h")
genieHNNieves.SetLineColor(ROOT.kGreen+2)
genieHNNieves.Scale(1., "width")

#TODO: turn this back on
genieHAEmpirical = genieHAEmpiricalFile.Get("pt")
genieHAEmpirical.SetTitle("GENIE v3 hA Empirical 2p2h")
genieHAEmpirical.SetLineColor(ROOT.kOrange+1)
genieHAEmpirical.Scale(1., "width")

genieHNEmpirical = genieHNEmpiricalFile.Get("pt")
genieHNEmpirical.SetTitle("GENIE v3 hN Empirical 2p2h")
genieHNEmpirical.SetLineColor(ROOT.kMagenta+2)
genieHNEmpirical.Scale(1., "width")

otherModels = [mnvTuneCrossSection, genieHANieves, genieHNNieves, genieHAEmpirical, genieHNEmpirical]

removeLowRecoilQEErrorBand(dataCrossSection)
formatHist(dataCrossSection)

for model in otherModels:
  #removeLowRecoilQEErrorBand(model)
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
plotter.error_color_map["NeutronInelasticExclusives"] = ROOT.kBlue
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
