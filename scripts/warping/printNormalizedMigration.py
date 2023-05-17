import ROOT
from ROOT import PlotUtils
import sys

fileToRead = ROOT.TFile.Open(sys.argv[1])
histName = "Tracker_MuonPTSignal_Migration"

maxX = 1
maxY = maxX

xTitle = "Reconstructed Muon Transverse Momentum [GeV/c]"
yTitle = "True Muon Transverse Momentum [GeV/c]"

def rowNormalize(hist):
  result = hist.Clone()
  nBinsX = result.GetXaxis().GetNbins()
  nBinsY = result.GetYaxis().GetNbins()

  for whichY in range(0, nBinsY + 1):
    rowSum = sum([result.GetBinContent(result.GetBin(thisX, whichY)) for thisX in range(0, nBinsX + 1)])
    if rowSum != 0:
      for whichX in range(0, nBinsX + 1):
        whichBin = result.GetBin(whichX, whichY)
        result.SetBinContent(whichBin, result.GetBinContent(whichBin)/rowSum)

  result.GetXaxis().SetTitle(xTitle)
  result.GetYaxis().SetTitle(yTitle)
  result.GetZaxis().SetTitle("Row Normalized")

  return result

def colNormalize(hist):
  result = hist.Clone()
  nBinsX = result.GetXaxis().GetNbins()
  nBinsY = result.GetYaxis().GetNbins()

  for whichX in range(0, nBinsX + 1):
    colSum = sum([result.GetBinContent(result.GetBin(whichX, thisY)) for thisY in range(0, nBinsY + 1)])
    if colSum != 0:
      for whichY in range(0, nBinsY + 1):
        whichBin = result.GetBin(whichX, whichY)
        result.SetBinContent(whichBin, result.GetBinContent(whichBin)/colSum)

  return result

ROOT.gStyle.SetOptStat(0)
ROOT.gStyle.SetPaintTextFormat("4.2g")

histToPlot = fileToRead.Get(histName)
histToPlot.GetXaxis().SetRangeUser(0, maxX)
histToPlot.GetYaxis().SetRangeUser(0, maxY)
can = ROOT.TCanvas("normalized")
can.SetRightMargin(0.15) #Make sure z axis label is visible
plotter = ROOT.PlotUtils.MnvPlotter()
ROOT.gStyle.SetPalette(ROOT.kBird) #MnvPlotter seems to override this :(

histToPlot.Scale(1./histToPlot.Integral())
histToPlot.Draw("colzTEXT")
plotter.WritePreliminary("TC", 0.035, 0, 0, True)
can.Print(fileToRead.GetName() + "_" + histName + "_areaNormalized.png")

rowNorm = rowNormalize(histToPlot)
rowNorm.Draw("colzTEXT")
plotter.WritePreliminary("TC", 0.035, 0, 0, True)
can.Print(fileToRead.GetName() + "_" + histName + "_rowNormalized.png")

colNorm = colNormalize(histToPlot)
colNorm.Draw("colzTEXT")
plotter.WritePreliminary("TC", 0.035, 0, 0, True)
can.Print(fileToRead.GetName() + "_" + histName + "_columnNormalized.png")
