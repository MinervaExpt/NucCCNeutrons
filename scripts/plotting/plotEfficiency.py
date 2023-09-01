import ROOT
from ROOT import PlotUtils

import sys

ROOT.gROOT.SetBatch() #Don't render histograms to a window.  Also gets filled areas correct.

ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetEndErrorSize(4)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetPadBottomMargin(0.12)
ROOT.gStyle.SetPadLeftMargin(0.13)
ROOT.gStyle.SetPadRightMargin(0.02)
ROOT.gStyle.SetPadTopMargin(0.07)
ROOT.gStyle.SetTitleSize(0.045, "xyz")
ROOT.gStyle.SetTitleOffset(1.25, "y")
ROOT.gStyle.SetTitleOffset(1.1, "x")
ROOT.gStyle.SetLabelSize(0.05, "xyz")
ROOT.gROOT.ForceStyle()

effFile = ROOT.TFile(sys.argv[1])

def removeLowRecoilQEErrorBand(hist):
  bandName = "Low_Recoil_2p2h_Tune"
  oldErrorBand = hist.PopVertErrorBand(bandName)
  if oldErrorBand:
    universes = oldErrorBand.GetHists()
    universes.erase(universes.begin() + 2)
    hist.AddVertErrorBand(bandName, universes)

can = ROOT.TCanvas("efficiency", "", 700, 500)

num = effFile.Get("Tracker_MuonPTSignal_EfficiencyNumerator")
denom = effFile.Get("Tracker_MuonPTSignal_EfficiencyDenominator")

num.Divide(num, denom)
removeLowRecoilQEErrorBand(num)

num.SetLineColor(ROOT.kRed)
num.SetLineWidth(3)
num.SetMarkerStyle(0)
num.SetMinimum(0)
num.SetMaximum(1)
num.GetXaxis().SetTitle("True Muon Transverse Momentum [GeV/c]") #Match format of paper
num.GetXaxis().SetTitleOffset(1.25) #Huh, this is getting overridden somewhere between the beginning of the script and here.  But the y axis isn't.  Why?
num.GetYaxis().SetTitle("Efficiency x Acceptance")
num.Draw("HIST")

totalErr = num.GetCVHistoWithError()
totalErr.SetFillColorAlpha(ROOT.kPink+1, 0.4)
#totalErr.SetFillStyle(1001)
totalErr.SetLineStyle(0)
totalErr.Draw("E2 SAME")
#plotter.WritePreliminary(0.4, 0.86, 5e-2, True)

can.Print("efficiency.png")
can.Print("efficiency.pdf")

plotter = PlotUtils.MnvPlotter()
plotter.ApplyStyle(PlotUtils.kCCQENuStyle)
plotter.error_color_map["NeutronInelasticExclusives"] = ROOT.kYellow+2

plotter.DrawErrorSummary(num)
#plotter.WritePreliminary(0.4, 0.86, 5e-2, True)
can.Print("efficiency_uncertaintyBreakdown.png")
