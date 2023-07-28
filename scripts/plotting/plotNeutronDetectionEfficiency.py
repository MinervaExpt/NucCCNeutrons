#USAGE: python plotNeutronDetectionEfficiency.py <MainCVFile.root> [otherModelFiles.root]
import ROOT
import sys

numName = "Tracker_Neutron_Detection_EfficiencyNumeratorEnergy"
denomName = "Tracker_Neutron_Detection_EfficiencyDenominatorEnergy"

mainFile = sys.argv[1]
otherFiles = sys.argv[2:]

ROOT.TH1.AddDirectory(False)
ROOT.gStyle.SetOptStat(0)
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetEndErrorSize(4)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetPadBottomMargin(0.12)
ROOT.gStyle.SetPadLeftMargin(0.13) #0.15)
ROOT.gStyle.SetPadRightMargin(0.02)
ROOT.gStyle.SetPadTopMargin(0.07) #0.12) #With title
ROOT.gStyle.SetTitleSize(0.045, "xyz") #0.055
ROOT.gStyle.SetTitleOffset(1.15, "y")
ROOT.gStyle.SetTitleOffset(1.1, "x")
ROOT.gStyle.SetLabelSize(0.05, "xyz")
ROOT.gStyle.SetTitleFont(42, "xyz")
ROOT.gROOT.ForceStyle()

def calcEfficiency(fileName, color):
  myFile = ROOT.TFile.Open(fileName, "READ")
  num = myFile.Get(numName).GetCVHistoWithStatError()
  denom = myFile.Get(denomName).GetCVHistoWithStatError()

  eff = num.Clone()
  eff.Divide(num, denom, 1., 1., "B") #Divide with binomial errors
  eff.SetLineWidth(3)
  eff.SetLineColor(color)
  afterFirstUnderscore = fileName.find("_")+1
  eff.SetTitle(fileName[afterFirstUnderscore:fileName.find("_", afterFirstUnderscore)]) #TODO: Get the interesting part out of the end of the file name
  eff.GetYaxis().SetTitle("Neutron Detection Efficiency")
  eff.GetXaxis().SetTitle("True Neutron Kinetic Energy [MeV]")
  return eff

canvas = ROOT.TCanvas("neutronDetectionEfficiency", "Neutron Detection Efficiency")
#plotter = ROOT.PlotUtils.MnvPlotter()
mainEff = calcEfficiency(mainFile, ROOT.kBlack)

#First, make the money plot: neutron detection efficiency for main model with no error bars (because I'm not confident in them at the moment)
oldTitle = mainEff.GetTitle()
mainEff.SetTitle("Neutron Detection Efficiency")
mainEff.SetMarkerStyle(0)
mainEff.Draw("HIST")
#plotter.WritePreliminary("TC", 0.035, 0, 0, True)
canvas.Print("neutronDetectionEfficiency.png")

#Now, compare different models
mainEff.GetYaxis().SetRangeUser(0, mainEff.GetMaximum()*1.1)
mainEff.Draw("HIST")

colors = ROOT.MnvColors.GetColors(ROOT.MnvColors.kOkabeItoDarkPalette)
whichColor = 0
efficiencies = [] #I need somewhere to keep these in memory lest python auto-delete them!

for otherFile in otherFiles:
  eff = calcEfficiency(otherFile, colors[whichColor])
  eff.Draw("HIST SAME")
  efficiencies.append(eff)
  whichColor = whichColor + 1

canvas.BuildLegend(0.5, 0.3, 0.9, 0.7)
#plotter.WritePreliminary("TC", 0.035, 0, 0, True)
canvas.Print("neutronDetectionEfficiency_modelComparison.png")

#Last, make neutron detection efficiency plot for the main model with an error envelope because I'm curious anyway.
mainEff.SetLineColor(ROOT.kRed)
errors = mainEff.Clone()
errors.SetFillColorAlpha(ROOT.kRed, 0.4)
errors.Draw("E2")
mainEff.Draw("HIST SAME")

#plotter.WritePreliminary("TC", 0.035, 0, 0, True)
canvas.Print("neutronDetectionEfficiency_withErrors.png")
canvas.Print("neutronDetectionEfficiency_withErrors.pdf")

#plotter.DrawErrorSummary(mainEff)
#plotter.WritePreliminary("TC", 0.035, 0, 0, True)
#canvas.Print("neutronDetectionEfficiency_errorBreakdown.png")
