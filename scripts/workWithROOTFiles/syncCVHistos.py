#!/bin/python
#Worry not if you forgot to call PlotUtils::HistWrapper::syncCVHistos().  MINERvA saves
#the whole histogram and doesn't calculate the covariance matrix until plotting.  So,
#this script can still save the day!
#USAGE: python syncCVHistos.py fileWithMnvH1Ds.root

import ROOT
from ROOT import PlotUtils
import sys
import shutil

#Makes sure the CV histogram copy in each error band matches the actual CV in an MnvH1D or MnvH2D.
#Important because each error band calculates its contribution to the covariance matrix
#independently of the CV itself.
#Modifies hist
#Copied from PlotUtils::HistWrapper<>
def syncCVHistos1D(hist):
  theCVHisto = ROOT.TH1D(hist)
  theCVHisto.SetDirectory(0)
  bandNames = hist.GetErrorBandNames()

  for bandName in bandNames:
    band = hist.GetVertErrorBand(bandName)
    theCVHisto.Copy(band)

def syncCVHistos2D(hist):
  theCVHisto = ROOT.TH2D(hist)
  theCVHisto.SetDirectory(0)
  bandNames = hist.GetErrorBandNames()

  for bandName in bandNames:
    band = hist.GetVertErrorBand(bandName)
    #band.operator=(theCVHisto)
    theCVHisto.Copy(band)

ROOT.TH1.AddDirectory(False)

originalFileName = sys.argv[1]
newFileName = originalFileName[:originalFileName.find(".root")] + "_withSyncCVHistos.root"

shutil.copy(originalFileName, newFileName)
fileToModify = ROOT.TFile(newFileName, "UPDATE")

listOfKeys = fileToModify.GetListOfKeys().Clone()
for key in listOfKeys:
  hist = key.ReadObj()
  try:
    syncCVHistos1D(hist)
    hist.Write(key.GetName(), ROOT.TObject.kOverwrite) #For some reason, hist.GetName() doesn't match key.GetName()?
  except:
    try:
      syncCVHistos2D(hist)
      hist.Write(key.GetName(), ROOT.TH1.kOverwrite) #For some reason, hist.GetName() doesn't match key.GetName()?
    except:
      print "Key named " + key.GetName() + " doesn't appear to be an MnvHND, so not touching it."

fileToModify.Write(newFileName, ROOT.TObject.kOverwrite)
