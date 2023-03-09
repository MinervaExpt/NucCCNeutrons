import ROOT
from ROOT import PlotUtils

import sys

ROOT.gROOT.SetBatch() #Don't render histograms to a window.  Also gets filled areas correct.

effFile = ROOT.TFile(sys.argv[1])

def removeLowRecoilQEErrorBand(hist):
  bandName = "Low_Recoil_2p2h_Tune"
  oldErrorBand = hist.PopVertErrorBand(bandName)
  if oldErrorBand:
    universes = oldErrorBand.GetHists()
    universes.erase(universes.begin() + 2)
    hist.AddVertErrorBand(bandName, universes)

can = ROOT.TCanvas("efficiency")
can.SetRightMargin(0.01)
can.SetLeftMargin(0.08)

plotter = PlotUtils.MnvPlotter()
plotter.ApplyStyle(PlotUtils.kCCQENuStyle)
plotter.error_color_map["NeutronInelasticExclusives"] = ROOT.kYellow+2

num = effFile.Get("Tracker_MuonPTSignal_EfficiencyNumerator")
denom = effFile.Get("Tracker_MuonPTSignal_EfficiencyDenominator")

num.Divide(num, denom)
removeLowRecoilQEErrorBand(num)

num.SetLineColor(ROOT.kRed)
num.SetLineWidth(3)
num.SetMinimum(0)
num.SetMaximum(1)
num.GetYaxis().SetTitle("Efficiency x Acceptance")
num.Draw("HIST")

totalErr = num.GetCVHistoWithError()
totalErr.SetFillColorAlpha(ROOT.kPink+1, 0.4)
#totalErr.SetFillStyle(1001)
totalErr.SetLineStyle(0)
totalErr.Draw("E2 SAME")
#plotter.WritePreliminary(0.4, 0.86, 5e-2, True)

can.Print("efficiency.png")

plotter.DrawErrorSummary(num)
#plotter.WritePreliminary(0.4, 0.86, 5e-2, True)
can.Print("efficiency_uncertaintyBreakdown.png")
