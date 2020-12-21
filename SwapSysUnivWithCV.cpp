//File: SwapSysUnivWithCV.cpp
//Brief: A program to make a systematic universe the CV histogram for warping studies.
//       Operates on all MnvH1Ds and MnvH2Ds in the input file.  May not work so well
//       if there's a flux histogram hanging around.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#define USAGE "SwapSysUnivWithCV <fileToWarp.root> <nameOfErrorBand> <indexOfUniverseWithinBand>"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvH2D.h"
#pragma GCC diagnostic pop

//ROOT includes
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"

//Feature needed with pre-ROOT6 PlotUtils.
#ifndef NCINTEX
#include "Cintex/Cintex.h"
#endif

//c++ includes
#include <iostream>

//Convince the STL to talk to TIter so I can use std::find_if()
namespace std
{
  template <>
  struct iterator_traits<TIter>
  {
    using value_type = TObject;
    using pointer = TObject*;
    using reference = TObject&;
    using iterator_category = forward_iterator_tag;
  };
}

//Think of this like the bash command line utility 'find dir -name "*"',
//but for TObjects in ROOT files.  It also requires that every object
//is of type OBJ.  If you're using this with multiple OBJs and performance
//matters, you should rewrite it.
template <class OBJ>
std::vector<OBJ*> find(TDirectory& dir)
{
  std::vector<OBJ*> found;

  auto& keys = *dir.GetListOfKeys();
  for(auto obj: keys)
  {
    auto key = static_cast<TKey*>(obj);
    auto asObj = dynamic_cast<OBJ*>(key->ReadObj());
    auto asDir = dynamic_cast<TDirectory*>(key->ReadObj());
    if(asObj) found.push_back(asObj);
    else if(asDir)
    {
      auto recurse = find<OBJ>(*asDir);
      found.insert(found.end(), recurse.begin(), recurse.end());
    }
  }

  return found;
}

//Rearrange an MnvH1D or MnvH2D so that a systematic universe
//is now the Central Value universe.
template <class MNVH>
MNVH* shiftCV(const MNVH* hist, const std::string& bandName, const int whichUniv)
{
  if(hist->GetErrorBandNames().size() > 1) std::cerr << "WARNING: Only shifting the CV.  SwapSysUnivWithCV ignores all other error bands!\n";

  //Find the universe histogram that will become the CV.
  auto band = hist->GetVertErrorBand(bandName.c_str());
  if(!band) throw std::runtime_error("Could not find an error band named " + bandName + " to become the new CV.\n");

  auto oldCVHist = band->GetHist(whichUniv);
  if(!oldCVHist) throw std::runtime_error("Found error band " + bandName + ", but it doesn't have a " + std::to_string(whichUniv) + "th universe.\n");

  using BASE = typename std::remove_const<typename std::remove_reference<decltype(*oldCVHist)>::type>::type;

  //Make my own copy of the CV-to-be that isn't managed by hist its MnvVertErrorBand.
  BASE* newCVHist = new BASE(*oldCVHist);
  newCVHist->SetDirectory(0);

  MNVH* shiftedHist = new MNVH(*newCVHist);
  shiftedHist->SetName(hist->GetName());
  shiftedHist->SetTitle(hist->GetTitle());

  delete newCVHist;
  newCVHist = nullptr;

  //Ignoring systematics because I'm only using this for warping studies

  //Tell all other error bands that this is the CV.
  //I lifted much of this code from Ben Messerly's MINERvA
  //Analysis Toolkit in HistWrapper.cxx.
  /*const auto allBandNames = shiftedHist->GetErrorBandNames();
  for(const auto& name: allBandNames) shiftedHist->GetVertErrorBand(name.c_str())->typename BASE::operator=(*newCVHist);*/

  return shiftedHist;
}

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  //Check arguments
  if(!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
  {
    std::cout << USAGE << "\n";
    return 0;
  }

  const std::string fileName = argv[1];
  const auto bandName = argv[2];
  int whichUniv = -1;

  try
  {
    whichUniv = std::stoi(argv[3]);
  }
  catch(const std::invalid_argument& /*e*/)
  {
    std::cerr << argv[3] << " is not an integer!\n\n" << USAGE << "\n";
    return 1;
  }

  auto inFile = TFile::Open(fileName.c_str(), "UPDATE"); //TODO: Clone() so I don't mess up the original?
  if(!inFile)
  {
    std::cerr << fileName << ": no such file or directory.\n\n" << USAGE << "\n";
    return 1;
  }

  const auto outFileName = fileName.substr(0, fileName.find('.')) + "_" + bandName + ".root";
  auto outFile = TFile::Open(outFileName.c_str(), "CREATE");
  if(!outFile)
  {
    std::cerr << "Couldn't create " << outFileName << " in the current directory.  Does it already exist?\n";
    return 2;
  }

  //Make the swap
  try
  {
    const auto all1D = find<PlotUtils::MnvH1D>(*inFile);
    outFile->cd();
    for(auto hist: all1D) shiftCV(hist, bandName, whichUniv);

    const auto all2D = find<PlotUtils::MnvH2D>(*inFile);
    outFile->cd();
    for(auto hist: all2D) shiftCV(hist, bandName, whichUniv);
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to swap the CV with " << bandName << " because:\n" << e.what() << "\n";
    return 3;
  }

  //Write the swapped histograms to a new file.
  //TODO: With the same directory structure
  outFile->Write();

  return 0;
}
