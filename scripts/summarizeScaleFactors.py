#USAGE: python summarizeScaleFactors.py before.root after.root
#Plots effective scale factors from a sideband fit procedure.

import ROOT
import sys

fiducialName = "Tracker"
varName = "MuonPT"
sidebandName = varName + "Signal" #Each background had better have the same ratio across all sidebands.  So, pick on the "sideband" that's always there: the selected region

beforeFile = ROOT.TFile.Open(sys.argv[1])
afterFile = ROOT.TFile.Open(sys.argv[2])

plotter = ROOT.PlotUtils.MnvPlotter() #Nota Bene: MnvColors seems to behave weirdly under pyROOT.  I have to do something that forces PlotUtils to load before I use it.  I might as well set up the MnvPlotter I'll need later.
colors = ROOT.MnvColors.GetColors(ROOT.MnvColors.kOkabeItoDarkPalette)

def drawFitRatio(histName, nextColor, legend):
  #print "Looking for a histogram named " + histName
  beforeHist = beforeFile.Get(histName)
  afterHist = afterFile.Get(histName)

  afterHist.Divide(afterHist, beforeHist, 1., 1., "B")
  afterHist.SetLineWidth(5)
  afterHist.SetLineColor(colors[nextColor])
  afterHist.SetMarkerStyle(20 + nextColor)
  afterHist.SetMarkerColor(afterHist.GetLineColor())
  afterHist.SetMarkerSize(2)


  afterHist.GetXaxis().SetTitle("Reconstructed Muon Transverse Momentum [GeV/c]")
  afterHist.GetXaxis().SetTitleSize(0.05)
  afterHist.GetXaxis().SetTitleOffset(0.9)
  afterHist.GetXaxis().SetLabelSize(0.04)

  afterHist.GetYaxis().SetTitle("Backgrounds After Fit / Before Fit")
  afterHist.GetYaxis().SetRangeUser(0, 2)
  afterHist.GetYaxis().SetTitleSize(0.05)
  afterHist.GetYaxis().SetTitleOffset(0.9)
  afterHist.GetYaxis().SetLabelSize(0.04)

  afterHist.Draw("HIST SAME ][")
  afterHist.Draw("P HIST SAME ][")
  legend.AddEntry(afterHist, afterHist.GetTitle())
  return nextColor + 1

#Draw the comparison
canvas = ROOT.TCanvas("", "", 700, 500)
canvas.SetLeftMargin(0.1)
canvas.SetRightMargin(0.02)
canvas.SetBottomMargin(0.1)

nextColor = 0

sortedKeys = beforeFile.GetListOfKeys()
sortedKeys.Sort(True)
legend = ROOT.TLegend(0.6, 0.25, 0.95, 0.49)
for key in sortedKeys:
  name = str(key.GetName())
  if name.find(fiducialName + "_" + sidebandName + "_" + "Background") > -1:
    nextColor = drawFitRatio(key.GetName(), nextColor, legend)

signalHistName = fiducialName + "_" + sidebandName + "_" + "SelectedMCEvents"
afterFile.Get(signalHistName).SetTitle("Signal")
nextColor = drawFitRatio(signalHistName, nextColor, legend)

#canvas.BuildLegend(0.6, 0.25, 0.95, 0.49)
legend.Draw()
#plotter.WritePreliminary("TC", 0.035, 0, 0, True)
canvas.Print("sidebandFitSummary.png")
