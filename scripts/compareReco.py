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
  hist.SetLineWidth(4)
  #hist.GetYaxis().SetTitle("#frac{d#sigma}{dp_{T #mu}} [cm^{2} * c / GeV / nucleon]")
  #hist.GetXaxis().SetTitle("Reconstructed p_{T #mu} [GeV/c]")
  hist.GetXaxis().SetTitle("Reconstructed Muon Transverse Momentum [GeV/#it{c}]")
  hist.GetYaxis().SetTitle("entries/GeV*#it{c}")

dataFile = ROOT.TFile.Open("crossSection_constrained.root")
dataDistribution = dataFile.Get("backgroundSubtracted")
dataDistribution.SetTitle("Data")
dataDistribution.SetLineColor(ROOT.kBlack)
dataPOT = 1.1134084e+21 #dataFile.Get("POTUsed").GetVal() #TODO: Write out POT in ExtractCrossSection?

otherModels = []
mcColors = ROOT.MnvColors.GetColors(ROOT.MnvColors.kOkabeItoDarkPalette)
nextColor = 0
for fileName in sys.argv[1:]:
  otherFile = ROOT.TFile.Open(fileName, "READ")
  otherPOT = 2.04371e+22 #4.7994378e+21 #otherFile.Get("POTUsed").GetVal() #TODO: Write out POT in ExtractCrossSection?
  #if fileName.find("extended2p2h") > 0:
  #  otherPOT = 2.04371e+22
  otherDistribution = otherFile.Get("backgroundSubtracted")
  print "Setting title for file " + fileName + " to " + fileName[fileName.rfind("_")+1:fileName.find(".root")]
  otherDistribution.SetTitle(fileName[fileName.rfind("_")+1:fileName.find(".root")])
  otherDistribution.SetLineColor(mcColors[nextColor])
  otherDistribution.Scale(dataPOT / otherPOT)
  otherModels.append(otherDistribution)

  nextColor = nextColor + 1

removeLowRecoilQEErrorBand(dataDistribution)
formatHist(dataDistribution)

for model in otherModels:
  removeLowRecoilQEErrorBand(model)
  formatHist(model)

#Draw histograms
can = ROOT.TCanvas("backgroundSubtracted")

#yMax = max([model.GetMaximum() for model in otherModels])
legend = ROOT.TLegend(0.65, 0.3, 0.9, 0.9)
whichModel = 1
for model in otherModels:
  #model.SetMaximum(yMax * 1.1)
  model.Scale(1., "width")
  model.SetMaximum(0.3e6)
  model.Draw("HIST SAME")
  legend.AddEntry(model, "Model " + str(whichModel))
  whichModel += 1

dataHist = dataDistribution.GetCVHistoWithError()
dataHist.Scale(1., "width")
dataHist.Draw("SAME")
legend.AddEntry(dataHist, "Data", "le")

#can.BuildLegend(0.6, 0.6, 0.9, 0.9) #TODO: Redacted model names for work-in-progress plot
otherModels[0].SetTitle("Background Subtracted")
plotter = PlotUtils.MnvPlotter()
plotter.WritePreliminary(0.4, 0.86, 5.5e-2, True)
legend.Draw()
can.Print("backgroundSubtractedComp.png")

#Draw uncertainty summary on the cross section extracted from data
plotter.ApplyStyle(PlotUtils.kCCQENuStyle)

#plotter.error_color_map["UnifiedCrossTalk"] = ROOT.kBlue

plotter.error_summary_group_map["Cross Section Models"].push_back("Low_Recoil_2p2h_Tune")

plotter.error_summary_group_map["Detector Response"].push_back("response_proton") #= ["response_proton", "reponse_em", "respone_other"]
plotter.error_summary_group_map["Detector Response"].push_back("response_em")
plotter.error_summary_group_map["Detector Response"].push_back("response_meson")
plotter.error_summary_group_map["Detector Response"].push_back("response_other")
plotter.error_summary_group_map["Detector Response"].push_back("UnifiedCrossTalk")
plotter.error_summary_group_map["Detector Response"].push_back("Muon_Energy_MINOS")
plotter.error_summary_group_map["Detector Response"].push_back("Muon_Energy_MINERvA")
plotter.error_summary_group_map["Detector Response"].push_back("BeamAngleX")
plotter.error_summary_group_map["Detector Response"].push_back("BeamAngleY")
plotter.error_summary_group_map["Detector Response"].push_back("Muon_Energy_Resolution")
plotter.error_color_map["Detector Response"] = ROOT.kOrange+1

#plotter.error_summary_group_map["more GEANT"] = ["GEANT_Neutron", "GEANT_Pion", "GEANT_Proton"]
plotter.error_summary_group_map["GEANT"].push_back("NeutronInelasticExclusives")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Neutron")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Pion")
plotter.error_summary_group_map["GEANT"].push_back("GEANT_Proton")

plotter.axis_maximum = 0.8
plotter.axis_title_offset_x = 0.8
plotter.axis_title_offset_y = 0.8
plotter.legend_offset_x = -0.32
plotter.legend_offset_y = 0.05
plotter.legend_text_size = 0.06
plotter.DrawErrorSummary(dataDistribution)
can.SetBottomMargin(0.11)
can.SetLeftMargin(0.1)
can.SetTopMargin(0.03)
can.SetRightMargin(0.03)
plotter.WritePreliminary(0.8, 0.86, 3.5e-2, True)
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
