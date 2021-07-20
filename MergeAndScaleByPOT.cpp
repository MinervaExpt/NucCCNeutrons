//File: mergeAndScaleByPOT.cpp
//Brief: Program to combine per-playlist (=MINERvA run period) files from ProcessAnaTuples
//       into one large file by Add()ing histograms and Merge()ing TParameters.  MINERvA's
//       Monte Carlo sample often overruns a strict scaling of MC to data POT, so MC files
//       for which a Data.root file is also available will be scaled by the data to MC POT
//       ratio.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#define USAGE \
"USAGE: mergeAndScaleByPOT <files.root>\n"\
"******************************** Explanation ********************************\n"\
"Combines 2 or more <files.root> into a .root file whose name is the name of\n"\
"the first argument to this program with \"_merged\" appended before its\n"\
"prefix.  If none of <files.root> begins with \"Data.root\" and there is a\n"\
"\"Data.root\" file in the same directory as the first command line argument,\n"\
"all histograms are scaled by the ratio of their file's POT to their matching\n"\
"data file's POT.  Fails if the output file already exists.  Supports xrootd\n"\
"for remote file access at places like Fermilab via the usual TNetXNGFile.\n"\
"************************************* I/O ***********************************\n"\
"Input: 1 or more ROOT files produced by ProcessAnaTuples\n"\
"Output: 1 .root file named after the first argument with \"_merged\" in its\n"\
"        name\n"\
"Errors: 0 is returned to the OS on success.  Anything else indicates\n"\
"        failure.  So, Makefiles for example can tell whether this program\n"\
"        succeeded and stop a graph if not.  Error messages are printed to\n"\
"        stderr only.\n"\
"*********************** Objects that Can be Merged **************************\n"\
"PlotUtils::MnvH1D\n"\
"PlotUtils::MnvH2D\n"\
"TH1D and any other ROOT histogram\n"\
"TParameter<double>\n"\
"Any TNamed will be ignored (but not objects derived from it)\n"\
"*****************************************************************************\n"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvH2D.h"

//ROOT includes
#include "TFile.h"
#include "TParameter.h"
#include "TH1.h"
#include "TSystem.h"
#include "TTree.h"

//Feature needed with pre-ROOT6 PlotUtils.
#ifndef NCINTEX
#include "Cintex/Cintex.h"
#endif

//c++ includes
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

enum errors
{
  success = 0,
  badCommandLine = 1,
  unknownFileObject = 2,
  badOutputFile = 3,
  inputFileFailed = 4,
  badMetadata = 5
};

std::string dirName(const std::string& fileName)
{
  return fileName.substr(0, fileName.rfind('/'));
}

std::string baseName(const std::string& fileName)
{
  const size_t lastSlash = fileName.rfind('/');
  return fileName.substr(lastSlash+1, fileName.find('.', lastSlash) - lastSlash - 1);
}

double getPOTScale(TFile& mcFile)
{
  //Get POT scale
  double scaleFactor = 1.;
  const std::string mcFileName = mcFile.GetName();
  const auto dataFileName = dirName(mcFileName) + "/" + baseName(mcFileName).substr(0, baseName(mcFileName).find("MC")) + "Data.root";
  std::unique_ptr<TFile> dataFile(TFile::Open(dataFileName.c_str(), "READ"));
  if(!dataFile) throw std::runtime_error(std::string("First file had a \"Data.root\" file in its directory, but I can't open a data file for ") + mcFileName + " called " + dataFileName + ".");
                                                                                                                                   
  const auto dataPOT = static_cast<TParameter<double>*>(dataFile->Get("POTUsed"));
  if(!dataPOT) throw std::runtime_error(std::string("Succeeded in opening ") + dataFileName + " to scale POT for " + mcFileName + "  because the first file had a \"Data.root\" file, but I can't get the POT from this data file.");
                                                                                                                                   
  scaleFactor = dataPOT->GetVal();
                                                                                                                                     
  const auto mcPOT = static_cast<TParameter<double>*>(mcFile.Get("POTUsed"));
  if(!mcPOT) throw std::runtime_error(std::string("Failed to find POT in ") + mcFileName + ".");

  return scaleFactor/mcPOT->GetVal();
}

bool checkMetadata(TFile& lhs, TFile& rhs)
{
  //Check NucCCNeutrons commit hash
  const auto lhsCommitHash = dynamic_cast<const TNamed*>(lhs.Get("NucCCNeutronsGitCommitHash")),
             rhsCommitHash = dynamic_cast<const TNamed*>(rhs.Get("NucCCNeutronsGitCommitHash"));
  if(strcmp(lhsCommitHash->GetName(), rhsCommitHash->GetName()))
  {
    std::cerr << lhs.GetName() << " has a different commit hash from " << rhs.GetName() << "\n";
    return false;
  }

  //Put PlotUtils commit hash etc. here.

  return true;
}

int main(const int argc, const char** argv)
{
  TH1::AddDirectory(kFALSE);

  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  {
    PlotUtils::MnvH1D pleaseDontOptimizeMe;
  }

  if(argc < 2) //argv[0] is always the path to this program
  {
    std::cerr << "At least 1 argument must be provided.\n\n" << USAGE;
    return badCommandLine;
  }

  const std::string firstFileName = argv[1],
                    outFileName = baseName(firstFileName) + "_merged.root";

  //Start with a copy of the first file
  const int copyStatus = gSystem->CopyFile(firstFileName.c_str(), outFileName.c_str(), kFALSE);
  std::unique_ptr<TFile> outFile(TFile::Open(outFileName.c_str(), "UPDATE"));

  if(copyStatus || !outFile)
  {
    std::cerr << "Failed to create a file named " << outFileName
              << ".  Copy exited with status " << copyStatus
              << ".  If the file already exists, I refuse to overwrite it.\n\n" << USAGE;
    return badOutputFile;
  }

  bool scaleToDataPOT = false;
  {
    const auto firstDataFileName = dirName(firstFileName) + "/" + baseName(firstFileName).substr(0, baseName(firstFileName).find("MC")) + "Data.root";
    std::unique_ptr<TFile> firstDataFile(TFile::Open(firstDataFileName.c_str(), "READ"));
    if(firstDataFile != nullptr) scaleToDataPOT = true;
    else std::cout << "WARNING: Not scaling MC to data POT.  Failed to find a data file, named " << firstDataFileName << ", to match " << firstFileName << ".\n";
  }

  //Scale the copied input file to the data POT
  if(scaleToDataPOT)
  {
    std::unique_ptr<TFile> firstFile(TFile::Open(firstFileName.c_str(), "READ"));
    const double scale = getPOTScale(*firstFile);

    for(auto entry: *outFile->GetListOfKeys())
    {
      auto key = static_cast<TKey*>(entry);
      auto th1 = dynamic_cast<TH1*>(key->ReadObj());
      if(th1)
      {
        th1->Scale(scale);
        //outFile->cd();
        //th1->Write(key->GetName());
      }
    }
  }
  outFile->SaveSelf(kTRUE);

  //Collect metadata from inFile
  const auto firstCommitHash = dynamic_cast<const TNamed*>(outFile->Get("NucCCNeutronsGitCommitHash"));
  if(!firstCommitHash)
  {
    std::cerr << "Failed to find a commit hash in " << outFileName << ".\n\n" << USAGE;
    return unknownFileObject;
  }

  for(int whichFile = 2; whichFile < argc; ++whichFile)
  {
    const std::string inFileName = argv[whichFile];

    std::cout << "Starting merge of " << inFileName << "\n";

    //TODO: Check for TNetXNGFile/xrootd read failures during merging.
    std::unique_ptr<TFile> inFile(TFile::Open(inFileName.c_str(), "READ"));
    if(!inFile)
    {
      std::cerr << "Failed to open a file named " << inFileName << " for reading.\n\n" << USAGE;
      return inputFileFailed;
    }

    double scale = 1.;
    if(scaleToDataPOT)
    {
      try { scale = getPOTScale(*inFile); }
      catch(const std::runtime_error& e)
      {
        std::cerr << "Failed to get POT scale for " << inFileName << " because:\n" << e.what() << "\n\n" << USAGE;
        return inputFileFailed;
      }
    }

    if(!checkMetadata(*outFile, *inFile)) return badMetadata;
    const int keysDiff = inFile->GetListOfKeys()->GetEntries() - outFile->GetListOfKeys()->GetEntries();
    if(keysDiff != 0) std::cout << "WARNING: " << inFileName << " has " << keysDiff << " keys that are not in " << firstFileName << ".  They will NOT be merged!  Continuing merging anyway...\n";

    //outFile->cd(); //TODO: Delete me.  Doesn't help.
    for(auto entry: *outFile->GetListOfKeys())
    {
      auto key = static_cast<TKey*>(entry);
      const auto obj = inFile->Get(key->GetName());
      auto mergeWith = outFile->Get(key->GetName()); //key->ReadObj();
      if(!obj)
      {
        std::cerr << "Found an object, " << key->GetClassName() << " " << key->GetName() << ", in " << firstFileName << " that is not in " << inFileName << ".\n\n" << USAGE;
        key->Print();
        return unknownFileObject;
      }

      //Do per-object merging.  Only merge objects I care about.  Fail if any other found.
      if(dynamic_cast<const TH1*>(obj)) //Covers both MnvH1/2D and TH1D
      {
        assert(dynamic_cast<TH1*>(mergeWith));
        static_cast<TH1*>(mergeWith)->Add(static_cast<const TH1*>(obj), scale);
      }
      else if(dynamic_cast<const TParameter<double>*>(obj))
      {
        assert(dynamic_cast<TParameter<double>>(mergeWith));
        std::cout << "mergeWith had value " << static_cast<TParameter<double>*>(mergeWith)->GetVal() << "\n";
        TList toMerge;
        toMerge.Add(obj);
        static_cast<TParameter<double>*>(mergeWith)->Merge(&toMerge);
        std::cout << "mergeWith now has value " << static_cast<TParameter<double>*>(mergeWith)->GetVal() << "\n";
      }
      else if(!strcmp(key->GetClassName(), "TNamed")) {} //Ignore these for now.  Could be playlist or version metadata
      else
      {
        std::cerr << "Found an object, " << key->GetClassName() << " " << key->GetName() << ", that I don't know how to merge.\n\n" << USAGE;
        return unknownFileObject;
      }
      //TODO: Recurse on TDirectories

      outFile->cd();
      mergeWith->Write(key->GetName()); //, TObject::kOverwrite); //Awesome, this leads to memory corruption...
      //key->WriteFile(1, outFile.get()); //TODO: Why does this crash?  I haven't found any good documentation for it yet.
    }
    outFile->SaveSelf(kTRUE); //Write();
  }

  return success;
}
