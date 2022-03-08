import ROOT
from ROOT import PlotUtils
import sys

bandToRemove = "NeutronInelasticExclusives"
inFile = ROOT.TFile.Open(sys.argv[1], "UPDATE")

for key in inFile.GetListOfKeys():
  try:
    key.ReadObj().PopVertErrorBand(bandToRemove)
  except:
    print "Key named " + key.GetName() + " doesn't appear to be an MnvHND, so not removing any error bands."

inFile.Write("", ROOT.TObject.kOverwrite)
