import ROOT
from ROOT import PlotUtils
import sys

prefix = "Tracker_MuonPTSignal_Denominator_"
interestingProcessName = "2p2h"
bottomCategoryName = prefix + interestingProcessName

ROOT.TH1.AddDirectory(False) #Stop MnvH1D's weird way of handling error band I/O from breaking things when I use Clone()
ROOT.gROOT.SetBatch(True)
ROOT.gStyle.SetOptStat(0)

inFile = ROOT.TFile(sys.argv[1])
bottomCategory = inFile.Get(bottomCategoryName).ProjectionY() #Put the most interesting category on the bottom so it's easy to read off ratios

endOfOriginalFile = inFile.GetName().rfind("MC_merged")
stack = ROOT.THStack("GENIEBreakdown", inFile.GetName()[inFile.GetName().find("_")+1:endOfOriginalFile] + " Process Breakdown")

#Format THStack axes in the weird way that it wants
axes = bottomCategory.Clone()
axes.SetMaximum(1.)
axes.GetXaxis().SetTitle("True Muon Transverse Momentum [GeV/c]")
axes.GetYaxis().SetTitle("Fraction of Total")
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
legend = ROOT.TLegend(0.75, 0.5, 0.98, 0.9, "", "brNDCF")
for hist in stack.GetStack():
  hist.SetLineWidth(0)
  hist.SetLineColor(colors[nextColor])
  hist.SetFillStyle(1001)
  hist.SetFillColor(colors[nextColor])
  hist.Divide(hist, categorySum)
  legend.AddEntry(hist)
  nextColor = nextColor + 1

#Also plot fraction of total content to give an idea of which bins are most important for the total rate.
fractionalContent = categorySum.Clone()
fractionalContent.Scale(1./categorySum.Integral())
fractionalContent.SetTitle("Shape")
fractionalContent.SetLineWidth(4)
fractionalContent.SetLineColor(ROOT.kBlack)
legend.AddEntry(fractionalContent)

#Make the actual plot
can = ROOT.TCanvas("GENIE Stack", "", 700, 500)
can.SetLeftMargin(0.085)
can.SetRightMargin(0.02)

stack.Draw("HIST")
fractionalContent.Draw("HIST SAME")
legend.Draw()
#prelim = ROOT.TLatex(0.2, 0.9, "#font[12]{ #color[3]{MINER#nuA Work in Progress} }")
prelim = ROOT.TLatex(0.2, 0.9, "MINER#nuA Work in Progress")
prelim.SetTextColor(ROOT.kRed+1)
prelim.SetTextFont(112)
prelim.Draw()
can.Print("GENIEBreakdown_" + inFile.GetName() + ".png")
