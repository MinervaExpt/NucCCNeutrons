import ROOT
import PlotUtils
import os,sys

ROOT.TH1.AddDirectory(1)


inputfile = ROOT.TFile(sys.argv[1])
stat_universe = sys.argv[2]

can = ROOT.TCanvas("c","c",10,10,1000,700)
myobjs = []

for i in range(1,15):
    myobj = inputfile.Get("Pull_Histograms/Stat_%s_Iter_%d_pull"%(stat_universe,i))
    myobj.SetLineColor(100-i*3)#Start red and go violet
    myobjs.append(myobj)


for i,o in enumerate(myobjs):
    if(i==0):
        o.Draw()
    else:
        o.Draw("SAME")

raw_input("Done")
