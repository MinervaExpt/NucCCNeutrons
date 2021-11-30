//File: SpecialSampleAsErrorBand.cpp
//Brief: A program to add an error band to a set of histograms using
//       the same histograms produced with a special (Gaudi) sample.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#define USAGE "SpecialSampleAsErrorBand <standardFile> <nameOfBand> <specialSampleFile> [additionalSpecialSampleUniverses]"

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

#include "TParameter.h"

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

template <class MNVH>
MNVH* cloneWithNewBand(const MNVH* hist, const std::string& bandName, const double cvPOT, const std::vector<std::unique_ptr<TFile>>& specialSamples)
{
  MNVH* withNewBand = new MNVH(*hist);

  using hist_t = typename std::remove_const<typename std::remove_pointer<decltype(hist->GetVertErrorBand(bandName)->GetHist(0))>::type>::type;
  std::vector<hist_t*> univs;
  for(auto& sample: specialSamples)
  {
    auto found = sample->Get<MNVH>(hist->GetName());
    if(!found) throw std::runtime_error(std::string("Failed to find a histogram named ") + hist->GetName() + " in " + sample->GetName());
    const auto specialPOT = sample->Get<TParameter<double>>("POTUsed");
    if(!specialPOT) throw std::runtime_error(std::string("Found histogram but failed to find POT in ") + sample->GetName());

    found->Scale(cvPOT / specialPOT->GetVal());
    univs.push_back(found); //Peel off just the CV by storing a pointer to the base class.
  }

  //Because we don't have a good procedure for 1-universe error bands,
  //create a second symmetric universe from the special sample universe
  //and the difference with the CV.
  if(univs.size() == 1)
  {
    const auto origUniv = univs.front();
    auto otherUniv = static_cast<hist_t*>(origUniv->Clone());
    otherUniv->SetDirectory(nullptr);
    otherUniv->Add(hist, -1.);
    otherUniv->Add(hist, otherUniv, 1., -1.);
    univs.push_back(otherUniv);
  }

  withNewBand->AddVertErrorBand(bandName, univs);

  return withNewBand;
}

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(false);

  //Check arguments
  if(argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") || argc < 4)
  {
    std::cout << USAGE << "\n";
    return 0;
  }

  const std::string fileName = argv[1];
  auto inFile = TFile::Open(fileName.c_str(), "READ");
  if(!inFile)
  {
    std::cerr << fileName << ": no such file or directory.\n\n" << USAGE << "\n";
    return 2;
  }

  const std::string newBandName = argv[2];
  std::vector<std::unique_ptr<TFile>> specialSamples;
  for(int whichSample = 3; whichSample < argc; ++whichSample)
  {
    auto file = TFile::Open(argv[whichSample], "READ");
    if(!file)
    {
      std::cerr << "Failed to open a special sample file named " << argv[whichSample] << "\n";
      return 2;
    } 
    specialSamples.emplace_back(file);
  }

  const auto all1D = find<PlotUtils::MnvH1D>(*inFile);
  const auto all2D = find<PlotUtils::MnvH2D>(*inFile);

  const auto allParameters = find<TParameter<double>>(*inFile);
  //const auto allStrings = find<TNamed>(*inFile);
  const auto cvPOT = inFile->Get<TParameter<double>>("POTUsed");
  if(!cvPOT)
  {
    std::cerr << "Failed to find POT information in CV file named " << inFile->GetName() << ".\n";
    return 2;
  }

  const size_t lastSlash = fileName.rfind('/');
  const auto outFileName = fileName.substr(lastSlash + 1, fileName.rfind('.') - lastSlash - 1) + "_with_" + newBandName + ".root";
  auto outFile = TFile::Open(outFileName.c_str(), "CREATE");
  if(!outFile)
  {
    std::cerr << "Couldn't create " << outFileName << " in the current directory.  Does it already exist?\n";
    return 4;
  }

  //Make the swap
  try
  {
    outFile->cd();
    for(const auto hist: all1D) cloneWithNewBand(hist, newBandName, cvPOT->GetVal(), specialSamples)->Write();
    for(const auto hist: all2D) cloneWithNewBand(hist, newBandName, cvPOT->GetVal(), specialSamples)->Write();
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to insert new error band, " << newBandName << ", because:\n" << e.what() << "\n";
    return 5;
  }

  //Copy TParameters (POT information) and TStrings (bookkeeping) to the new file.
  for(auto par: allParameters) par->Write();
  //for(auto str: allStrings) str->Write(); //TODO: I need all TNamed that aren't also TH1s

  //Write the swapped histograms to a new file.
  //TODO: With the same directory structure
  outFile->Write();

  return 0;
}
