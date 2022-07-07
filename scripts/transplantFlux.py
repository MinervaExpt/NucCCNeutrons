import ROOT
from ROOT import PlotUtils
import sys
import shutil

fileToModify = sys.argv[1] #= ROOT.TFile.Open(sys.argv[1], "UPDATE")
fluxFile = ROOT.TFile.Open(sys.argv[2])

newFileName = fileToModify[:fileToModify.find(".root")] + "_withFluxFrom_" + fluxFile.GetName() + ".root"
shutil.copy(fileToModify, newFileName)
newFile = ROOT.TFile.Open(newFileName, "UPDATE")

fluxName = "Tracker_MuonPTSignal_reweightedflux_integrated"

fluxFile.Get(fluxName).Write("", ROOT.TObject.kOverwrite)
newFile.Write("", ROOT.TObject.kOverwrite)
