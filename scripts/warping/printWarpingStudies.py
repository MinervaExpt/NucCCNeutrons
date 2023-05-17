import ROOT
from ROOT import PlotUtils
import sys

chi2SummaryDir = "Chi2_Iteration_Dists"
chi2SummaryName = "h_chi2_modelData_trueData_iter_chi2"
medianHistName = "h_median_chi2_modelData_trueData_iter_chi2"
meanChi2ProfileName = "m_avg_chi2_modelData_trueData_iter_chi2_truncated"

lineWidth = 3
iterChosen = 3

can = ROOT.TCanvas("chi2")

for fileName in sys.argv[1:]:
  myFile = ROOT.TFile.Open(fileName)

  #Try to infer a useful universe name from the file name
  univName = fileName[fileName.find("merged") + len("merged") + 1:fileName.find(".root")]
  if fileName.find("SuSA") != -1: #The SuSA warp is a stand-alone CV, so it needs special treatment
    univName = "SuSA"

  spread = myFile.Get(chi2SummaryDir).Get(chi2SummaryName)

  #Infer number of degrees of freedom from y axis title
  axisTitle = spread.GetYaxis().GetTitle()
  yNDF = int(axisTitle[axisTitle.find("ndf=") + 4:axisTitle.find(")")])

  spread.SetTitle("Universe: " + univName)
  spread.SetTitleOffset(0.75, "X")
  spread.SetTitleOffset(0.65, "Y")
  spread.Draw("colz")

  profile = myFile.Get(chi2SummaryDir).Get(meanChi2ProfileName)
  profile.SetTitle("Mean Chi2")
  profile.SetLineWidth(lineWidth)
  profile.SetLineColor(ROOT.kBlue)
  profile.SetMarkerStyle(0)
  profile.Draw("SAME")

  median = myFile.Get(chi2SummaryDir).Get(medianHistName)
  median.SetTitle("Median Chi2")
  median.SetLineWidth(lineWidth)
  median.SetLineColor(ROOT.kBlack)
  median.Draw("HIST SAME")

  #Draw lines at number of degrees of freedom and 2x NDF
  ndfLine = ROOT.TLine(1, yNDF, spread.GetXaxis().GetXmax(), yNDF)
  ndfLine.SetLineWidth(lineWidth)
  ndfLine.SetLineStyle(ROOT.kDashed)
  ndfLine.Draw()

  doubleNDFLine = ROOT.TLine(1, 2*yNDF, spread.GetXaxis().GetXmax(), 2*yNDF)
  doubleNDFLine.SetLineColor(ROOT.kRed)
  doubleNDFLine.SetLineWidth(lineWidth)
  doubleNDFLine.SetLineStyle(ROOT.kDashed)
  doubleNDFLine.Draw()

  #Draw a line at the chosen number of iterations.
  iterLine = ROOT.TLine(iterChosen + 0.5, 0, iterChosen + 0.5, spread.GetYaxis().GetXmax())
  iterLine.SetLineWidth(lineWidth)
  iterLine.SetLineStyle(ROOT.kDotted)
  iterLine.Draw()

  #Make a custom legend because I don't want to include the 2D histogram
  #while I must Draw() it first to set the right axis limits.
  leg = ROOT.TLegend(0.6, 0.6, 0.9, 0.9)
  leg.AddEntry(profile)
  leg.AddEntry(median)
  leg.AddEntry(ndfLine, "Number of Bins", "l")
  leg.AddEntry(doubleNDFLine, "2x Number of Bins", "l")
  leg.AddEntry(iterLine, str(iterChosen) + " iterations", "l")
  leg.Draw()

  can.Print(fileName + ".png")
