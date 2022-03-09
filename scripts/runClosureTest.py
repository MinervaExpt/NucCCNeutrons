#!/bin/python
#USAGE: runClosureTest.py </path/to/tuples.root>

import sys
import ROOT

if len(sys.argv) != 2:
  print "Path to tuples to compare is required."
  print "USAGE: runClosureTest.py </path/to/tuples.root>"
  sys.exit(1)

tuplePath = sys.argv[1]

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


#Run tuples using the current installation
ProcessAnaTuples multiNeutron.yaml ${TUPLE_PATH} &> crossSectionHists.txt &
runCCIncForMultiNeutron ${TUPLE_PATH} &> closureHists.txt &

wait #Wait for both processes to complete
#TODO: Save process numbers and use wait to check their exit codes

ExtractCrossSection 1 multiNeutronMC.root multiNeutronMC.root
#TODO: Make the three closure test comparisons
