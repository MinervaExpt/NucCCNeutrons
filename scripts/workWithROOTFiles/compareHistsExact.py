#compareHistsExact.py: compares 2 files of histograms.  Looks for each histogram by name and requires that each bin has the exact same number of entries.
#                      Use only when you make a change that should never affect physics.
#Exit codes:
#0: Comparison OK
#1: Some histograms had different contents
#2: Some histograms had different errors
#3: Some histograms were missing from rhsFile
#4: rhsFile had extra histograms
#5: Histograms of the same name could not be compared because they had a different number of bins, different class names, or different error band names.

import sys
import ROOT
from ROOT import PlotUtils

tolerance = 1e-7 #1e-11

#Read file names from the command line
if len(sys.argv) < 3 or len(sys.argv) > 4:
  print "compareHistsExact <lhsFileName.root> <rhsFileName.root> [tolerance]"
  print "Compares histograms between exactly 2 files.  Fails unless contents match to tolerance (default = " + str(tolerance) + ")"

lhsFile = ROOT.TFile.Open(sys.argv[1])
rhsFile = ROOT.TFile.Open(sys.argv[2])
if len(sys.argv) > 3:
  tolerance = float(sys.argv[3])

failedComp = 0 #TODO: Do this with bit fields instead so that combinations of failure modes are obvious from the command line?

def compareHists(lhs, rhs):
  if lhs.GetNcells() != rhs.GetNcells():
    print "Histogram " + lhs.GetName() + " has a different number of bins in each file.  " + str(lhs.GetNcells()) + " in LHS and " + str(rhs.GetNcells()) + " in RHS."
    return 5

  if lhs.ClassName() != rhs.ClassName():
    print "Histogram " + lhs.GetName() + " has a different class name in RHS, so it probably has different dimensionality."
    return 5

  for whichBin in range(lhs.GetNcells()+1):
    if abs(lhs.GetBinContent(whichBin) - rhs.GetBinContent(whichBin)) > tolerance:
      print "Histogram " + lhs.GetName() + " has different contents in bin " + str(whichBin) + ": " + str(lhs.GetBinContent(whichBin)) + " versus " + str(rhs.GetBinContent(whichBin))
      return 1
   
    if abs(lhs.GetBinError(whichBin) - rhs.GetBinError(whichBin)) > tolerance:
      print "Histogram " + lhs.GetName() + " has different sumw2 in bin " + str(whichBin) + ": " + str(lhs.GetBinError(whichBin)) + " versus " + str(rhs.GetBinError(whichBin))
      return 2

  return 0

for key in rhsFile.GetListOfKeys():
  if lhsFile.GetListOfKeys().FindObject(key.GetName()) == None:
    print "Found an extra histogram in " + rhsFile.GetName() + " named " + key.GetName()
    failedComp = 4

for key in lhsFile.GetListOfKeys():
  hist = key.ReadObj()

  foundKey = rhsFile.GetListOfKeys().FindObject(hist.GetName())
  if foundKey == None:
    print "Object named " + str(hist.GetName()) + " is missing from file " + rhsFile.GetName()
    failedComp = 3
  else:
    foundHist = foundKey.ReadObj()
    if key.GetClassName() == "PlotUtils::MnvH1D" or key.GetClassName() == "PlotUtils::MnvH2D": #N.B.: class name of found object in RHS is checked in compareHists
      for bandName in hist.GetVertErrorBandNames(): #TODO: Also loop lateral error bands for legacy files
        band = hist.GetVertErrorBand(bandName)
        foundBand = foundHist.GetVertErrorBand(bandName)

        #Compare the CVs first
        compStatus = compareHists(band, foundBand)
        if compStatus != 0:
          failedComp = compStatus

        #Compare each Universe in each MnvVertErrorBand
        for whichUniv in range(band.GetNHists()):
          univHist = band.GetHist(whichUniv)
          univFoundHist = foundBand.GetHist(whichUniv)
          compStatus = compareHists(univHist, univFoundHist)
          if compStatus != 0:
            failedComp = compStatus

    elif key.InheritsFrom("TH1"):
      compStatus = compareHists(hist, foundHist)
      if compStatus != 0:
        failedComp = compStatus

    else:
      if hist.ClassName() != foundHist.ClassName():
        print "Non-histogram object " + hist.GetName() + " has a different class name in RHS: " + foundHist.ClassName()
        failedComp = 5

sys.exit(failedComp)
