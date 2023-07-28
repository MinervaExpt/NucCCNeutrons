import ROOT
from ROOT import PlotUtils
import sys

prefix = "Tracker_MuonPTSignal_Denominator_"
interestingProcessName = "2p2h"
bottomCategoryName = prefix + interestingProcessName

ROOT.TH1.AddDirectory(False) #Stop MnvH1D's weird way of handling error band I/O from breaking things when I use Clone()
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetEndErrorSize(4)
ROOT.gStyle.SetOptTitle(0)
ROOT.gStyle.SetPadBottomMargin(0.12)
ROOT.gStyle.SetPadLeftMargin(0.13) #0.15)
ROOT.gStyle.SetPadRightMargin(0.02)
ROOT.gStyle.SetPadTopMargin(0.07) #0.12) #With title
ROOT.gStyle.SetTitleSize(0.045, "xyz") #0.055
ROOT.gStyle.SetTitleOffset(1.25, "y")
ROOT.gStyle.SetTitleOffset(1.1, "x")
ROOT.gStyle.SetLabelSize(0.05, "xyz")
ROOT.gROOT.ForceStyle()

inFile = ROOT.TFile(sys.argv[1])
bottomCategory = inFile.Get(bottomCategoryName).ProjectionY() #Put the most interesting category on the bottom so it's easy to read off ratios

endOfOriginalFile = inFile.GetName().rfind("MC_merged")
stack = ROOT.THStack("GENIEBreakdown", inFile.GetName()[inFile.GetName().find("_")+1:endOfOriginalFile] + " Process Breakdown")

#Format THStack axes in the weird way that it wants
axes = bottomCategory.Clone()
axes.SetMaximum(1.)
axes.GetXaxis().SetTitle("True Muon Transverse Momentum [GeV/c]")
axes.GetYaxis().SetTitle("Simulated Events")
stack.SetHistogram(axes)

stack.Add(bottomCategory)
categorySum = bottomCategory.Clone()

#Find the histogram for each GENIE category.
for key in inFile.GetListOfKeys():
  if key.GetName().find(prefix) > -1 and key.GetName().find(bottomCategoryName) < 0:
    otherCategory = key.ReadObj().ProjectionY()
    stack.Add(otherCategory)
    categorySum.Add(otherCategory)

#Format histograms
colors = ROOT.MnvColors.GetColors(ROOT.MnvColors.kOkabeItoDarkPalette)
nextColor = 0
for hist in stack.GetStack():
  hist.SetLineWidth(0)
  hist.SetLineColor(colors[nextColor])
  hist.SetFillStyle(1001)
  hist.SetFillColor(colors[nextColor])
  nextColor = nextColor + 1

#Build the legend so that its order goes from top to bottom, not bottom to top.
legend = ROOT.TLegend(0.75, 0.65, 0.98, 0.9, "", "brNDCF")

for whichHist in range(stack.GetStack().GetEntries()-1, -1, -1): #Iterate in reverse to get the TLegend order to match the order in which histograms were drawn :(
  hist = stack.GetStack()[whichHist]
  if whichHist > 0 and (hist.Integral() - stack.GetStack()[whichHist-1].Integral()) / categorySum.Integral() > 0.01:
    legend.AddEntry(hist, "", "f")
  elif whichHist == 0 and hist.Integral() / categorySum.Integral() > 0.01:
    legend.AddEntry(hist, "", "f")

#Make the actual plot
can = ROOT.TCanvas("GENIE Stack", "", 700, 500)

stack.Draw("HIST")
legend.Draw()
#prelim = ROOT.TLatex(0.2, 0.9, "#font[12]{ #color[3]{MINER#nuA Work in Progress} }")
can.Print("GENIEBreakdown_" + inFile.GetName() + ".png")
can.Print("GENIEBreakdown_" + inFile.GetName() + ".pdf")
