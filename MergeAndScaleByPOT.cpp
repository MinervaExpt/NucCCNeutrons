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
#include "TKey.h"

//Feature needed with pre-ROOT6 PlotUtils.
#ifndef NCINTEX
#include "Cintex/Cintex.h"
#endif

//c++ includes
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <numeric>

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
  const size_t lastSlash = fileName.rfind('/');
  if(lastSlash == std::string::npos) return ".";
  return fileName.substr(0, lastSlash);
}

std::string baseName(const std::string& fileName)
{
  const size_t lastSlash = fileName.rfind('/');
  return fileName.substr(lastSlash+1, fileName.find('.', lastSlash) - lastSlash - 1);
}

double getDataPOT(TFile& mcFile)
{
  //Get data POT corresponding to the playlist of mcFile
  const std::string mcFileName = mcFile.GetName();
  const auto dataFileName = dirName(mcFileName) + "/" + baseName(mcFileName).substr(0, baseName(mcFileName).find("MC")) + "Data.root";
  std::unique_ptr<TFile> dataFile(TFile::Open(dataFileName.c_str(), "READ"));
  if(!dataFile) throw std::runtime_error(std::string("First file had a \"Data.root\" file in its directory, but I can't open a data file for ") + mcFileName + " called " + dataFileName + ".");
                                                                                                                                   
  const auto dataPOT = static_cast<TParameter<double>*>(dataFile->Get("POTUsed"));
  if(!dataPOT) throw std::runtime_error(std::string("Succeeded in opening ") + dataFileName + " to scale POT for " + mcFileName + "  because the first file had a \"Data.root\" file, but I can't get the POT from this data file.");
                                                                                                                                   
  return dataPOT->GetVal();
}

double getMyPOT(TFile& mcFile)
{
  const auto myPOT = static_cast<TParameter<double>*>(mcFile.Get("POTUsed"));
  if(!myPOT) throw std::runtime_error(std::string("Failed to find POT in ") + mcFile.GetName() + ".");

  return myPOT->GetVal();
}

bool checkMetadata(TFile& lhs, TFile& rhs, const std::map<std::string, PlotUtils::MnvH1D*>& fiducials)
{
  //Check NucCCNeutrons commit hash
  const auto lhsCommitHash = dynamic_cast<const TNamed*>(lhs.Get("NucCCNeutronsGitCommitHash")),
             rhsCommitHash = dynamic_cast<const TNamed*>(rhs.Get("NucCCNeutronsGitCommitHash"));

  if(!rhsCommitHash)
  {
    std::cerr << rhs.GetName() << " doesn't have a commit hash!\n";
    return false;
  }

  if(strcmp(lhsCommitHash->GetName(), rhsCommitHash->GetName()))
  {
    std::cerr << lhs.GetName() << " has a different commit hash from " << rhs.GetName() << "\n";
    return false;
  }

  //Check playlist
  const auto lhsPlaylist = dynamic_cast<const TNamed*>(lhs.Get("playlist")),
             rhsPlaylist = dynamic_cast<const TNamed*>(rhs.Get("playlist"));
  if(!rhsPlaylist)
  {
    std::cerr << rhs.GetName() << " doesn't have a playlist!\n";
    return false;
  }

  if(strcmp(lhsPlaylist->GetName(), rhsPlaylist->GetName()))
  {
    std::cerr << lhs.GetName() << " is from playlist " << lhsPlaylist->GetName() << ", but " << rhs.GetName() << " is from playlist " << rhsPlaylist->GetName();
    return false;
  }

  //Check number of target nucleons ~= fiducial volume
  for(auto fiducial: fiducials)
  {
    const auto rhsFiducial = dynamic_cast<const PlotUtils::MnvH1D*>(rhs.Get(fiducial.first.c_str()));
    if(!rhsFiducial)
    {
      std::cerr << rhs.GetName() << " doesn't have a number of nucleons for " << fiducial.first << "\n";
      return false;
    }

    if(rhsFiducial->GetBinContent(1) != fiducial.second->GetBinContent(1))
    {
      std::cerr << rhs.GetName() << " has a different number of target nucleons from " << lhs.GetName() << ".  They probably came from processing different fiducial volumes!\n";
      return false;
    }
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

  bool scaleToDataPOT = false;
  {
    const auto firstDataFileName = dirName(firstFileName) + "/" + baseName(firstFileName).substr(0, baseName(firstFileName).find("MC")) + "Data.root";
    std::unique_ptr<TFile> firstDataFile(TFile::Open(firstDataFileName.c_str(), "READ"));
    if(firstDataFile != nullptr) scaleToDataPOT = true;
    else std::cout << "WARNING: Not scaling MC to data POT.  Failed to find a data file, named " << firstDataFileName << ", to match " << firstFileName << ".\n";
  }

  //Copy input file's objects into memory.  I need to keep my own copy
  //because even TFileDirectory::Get() seems to be overwriting the
  //memory object by whatever is in the file when I open another file
  //in between uses.
  //Keep number of nucleons, POT, and flux separate because they need to be merged differently.
  std::map<std::string, TObject*> mergedSamples;
  std::map<std::string, PlotUtils::MnvH1D*> mergedNucleons;
  std::map<std::string, TParameter<double>*> mergedPOT;
  std::map<std::string, TH1*> mergedFlux;
  std::map<std::string, TNamed*> mergedMetadata;
  std::unique_ptr<TFile> firstFile(TFile::Open(firstFileName.c_str(), "READ"));

  for(auto entry: *firstFile->GetListOfKeys())
  {
    auto key = static_cast<TKey*>(entry);
    const std::string keyName = key->GetName();
    auto obj = key->ReadObj()->Clone();

    if(keyName.find("_FiducialNucleons") != std::string::npos)
    {
      auto hist = dynamic_cast<PlotUtils::MnvH1D*>(obj);
      assert(hist && "Number of nucleons is not an MnvH1D!");
      mergedNucleons[keyName] = hist;
    }
    else if(!strcmp(key->GetName(), "POTUsed"))
    {
      auto param = dynamic_cast<TParameter<double>*>(obj);
      assert(param && "POTUsed is not a TParameter<double>!");
      mergedPOT[keyName] = param;
    }
    else if(keyName.find("_reweightedflux_integrated") != std::string::npos)
    {
      auto hist = dynamic_cast<TH1*>(obj);
      assert(hist && "Flux is not derived from TH1!");
      hist->SetDirectory(nullptr);
      mergedFlux[keyName] = hist;
    }
    else
    {
      if(strcmp(obj->ClassName(), "TNamed"))
      {
        if(dynamic_cast<TH1*>(obj)) static_cast<TH1*>(obj)->SetDirectory(nullptr);
        mergedSamples[keyName] = obj;
      }
      else mergedMetadata[keyName] = static_cast<TNamed*>(obj);
    }
  }

  //Ensure that metadata is in expected format in firstFile.
  //Otherwise, checkMetadata() comparisons later might crash!
  if(!checkMetadata(*firstFile, *firstFile, mergedNucleons)) return badMetadata;

  std::map<std::string, double> playlistToDataPOT;
  double totalMCPOT = 0;

  //Scale the copied input file to the data POT
  if(scaleToDataPOT)
  {
    try
    {
      const double dataPOT = getDataPOT(*firstFile);
      playlistToDataPOT[firstFile->Get<TNamed>("playlist")->GetTitle()] = dataPOT;
      const double mcPOT = getMyPOT(*firstFile);
      const double scale = dataPOT / mcPOT;

      std::cout << "Scaling by " << dataPOT << " / " << mcPOT << " = " << scale << " for file " << firstFile->GetName() << "\n";

      for(auto& entry: mergedSamples)
      {
        /*auto th1 = dynamic_cast<TH1*>(entry.second);
        if(th1) th1->Scale(scale);*/

        if(dynamic_cast<PlotUtils::MnvH1D*>(entry.second)) static_cast<PlotUtils::MnvH1D*>(entry.second)->Scale(scale);
        else
        {
          assert(dynamic_cast<PlotUtils::MnvH2D*>(entry.second));
          static_cast<PlotUtils::MnvH2D*>(entry.second)->Scale(scale);
        }
      }

      totalMCPOT = mcPOT;
    }
    catch(const std::exception& e)
    {
      std::cerr << "Failed to scale " << firstFile->GetName() << "'s histograms because:\n" << e.what() << "\n\n" << USAGE;
      return badMetadata;
    }
  }

  //Get just the first file's POT
  const double firstPlaylistPOT = std::accumulate(playlistToDataPOT.begin(), playlistToDataPOT.end(), 0., [](double sum, const auto& pair) { return sum + pair.second; });

  for(auto flux: mergedFlux)
  {
    if(dynamic_cast<PlotUtils::MnvH1D*>(flux.second))
    {
      static_cast<PlotUtils::MnvH1D*>(flux.second)->Scale(firstPlaylistPOT);
    }
    else
    {
      assert(dynamic_cast<PlotUtils::MnvH2D*>(flux.second));
      static_cast<PlotUtils::MnvH2D*>(flux.second)->Scale(firstPlaylistPOT);
    }
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

    double scale = 1., dataPOT = 0., mcPOT = 0.;
    const std::string playlistName = inFile->Get<TNamed>("playlist")->GetTitle();
    const auto foundPOT = playlistToDataPOT.find(playlistName);
    if(scaleToDataPOT)
    {
      try
      {
        if(foundPOT == playlistToDataPOT.end()) playlistToDataPOT[playlistName] = getDataPOT(*inFile);
        dataPOT = playlistToDataPOT[playlistName];
        mcPOT = getMyPOT(*inFile);
        scale = dataPOT / mcPOT;
      }
      catch(const std::runtime_error& e)
      {
        std::cerr << "Failed to scale " << inFileName << "'s histograms because:\n" << e.what() << "\n\n" << USAGE;
        return inputFileFailed;
      }
    }
    else
    {
      if(foundPOT == playlistToDataPOT.end()) playlistToDataPOT[playlistName] = getMyPOT(*inFile);
      dataPOT = playlistToDataPOT[playlistName];
    }

    std::cout << "Scaling by " << dataPOT << " / " << mcPOT << " = " << scale << " for file " << inFile->GetName() << "\n";

    totalMCPOT += mcPOT;

    if(!checkMetadata(*firstFile, *inFile, mergedNucleons)) return badMetadata;

    //Merge histograms
    for(auto& entry: mergedSamples)
    {
      const auto obj = inFile->Get(entry.first.c_str());
      auto mergeWith = entry.second;
      if(!obj)
      {
        std::cerr << "Found an object, " << mergeWith->ClassName() << " " << entry.first << ", in " << firstFileName << " that is not in " << inFileName << ".\n\n" << USAGE;
        return unknownFileObject;
      }

      //Do per-object merging.  Only merge objects I care about.  Fail if any other found.
      /*if(dynamic_cast<const TH1*>(obj)) //Covers both MnvH1/2D and TH1D
      {
        assert(dynamic_cast<TH1*>(mergeWith));
        static_cast<TH1*>(obj)->Scale(scale);
        static_cast<TH1*>(mergeWith)->Add(static_cast<const TH1*>(obj));
        //static_cast<TH1*>(mergeWith)->Add(static_cast<const TH1*>(obj), scale);
      }*/
      if(dynamic_cast<const PlotUtils::MnvH1D*>(obj))
      {
        assert(dynamic_cast<PlotUtils::MnvH1D*>(mergeWith));
        static_cast<PlotUtils::MnvH1D*>(mergeWith)->Add(static_cast<const PlotUtils::MnvH1D*>(obj), scale);
      }
      else if(dynamic_cast<const PlotUtils::MnvH2D*>(obj))
      {
        assert(dynamic_cast<PlotUtils::MnvH2D*>(mergeWith));
        static_cast<PlotUtils::MnvH2D*>(mergeWith)->Add(static_cast<const PlotUtils::MnvH2D*>(obj), scale);
      }
      else
      {
        std::cerr << "Found an object, " << mergeWith->ClassName() << " " << entry.first << ", that I don't know how to merge.\n\n" << USAGE;
        return unknownFileObject;
      }
      //TODO: Recurse on TDirectories.  My simple map of object names to TObjects probably won't work well with TDirectory anyway.
    }

    //Merge POT
    for(auto& pot: mergedPOT)
    {
      const auto param = dynamic_cast<TParameter<double>*>(inFile->Get(pot.first.c_str()));
      if(!param)
      {
        std::cerr << "POT in " << inFile->GetName() << " either doesn't exist or is not a TParameter<double>!\n\n" << USAGE;
        return unknownFileObject;
      }
      pot.second->SetVal(pot.second->GetVal() + param->GetVal());
    }

    //Merge flux
    for(auto& flux: mergedFlux)
    {
      /*const auto hist = dynamic_cast<PlotUtils::MnvH1D*>(inFile->Get(flux.first.c_str()));
      if(!hist)
      {
        std::cerr << "Flux histogram at " << flux.first << " in " << inFile->GetName() << " either doesn't exist or is not derived from TH1!\n\n" << USAGE;
        return unknownFileObject;
      }
      hist->Scale(dataPOT);
      flux.second->Add(hist);*/
      //flux.second->Add(hist, dataPOT); //I'll divide by total data POT at the end of this program

      const auto obj = inFile->Get(flux.first.c_str());
      if(dynamic_cast<const PlotUtils::MnvH1D*>(obj))
      {
        assert(dynamic_cast<PlotUtils::MnvH1D*>(flux.second));
        static_cast<PlotUtils::MnvH1D*>(flux.second)->Add(static_cast<const PlotUtils::MnvH1D*>(obj), dataPOT);
      }
      else if(dynamic_cast<const PlotUtils::MnvH2D*>(obj))
      {
        assert(dynamic_cast<PlotUtils::MnvH2D*>(flux.second));
        static_cast<PlotUtils::MnvH2D*>(flux.second)->Add(static_cast<const PlotUtils::MnvH2D*>(obj), dataPOT);
      }
      else
      {
        std::cerr << "Flux histogram at " << flux.first << " in " << inFile->GetName() << " either doesn't exist or is not derived from MnvH1D or MnvH2D!\n\n" << USAGE;
      }
    }
  }

  //Save memory objects into outFile
  std::unique_ptr<TFile> outFile(TFile::Open(outFileName.c_str(), "CREATE"));
                                                                                          
  if(!outFile)
  {
    std::cerr << "Failed to create a file named " << outFileName
              << ".  If the file already exists, I refuse to overwrite it.\n\n" << USAGE;
    return badOutputFile;
  }

  //Only count each data file once even if I have multiple files from the same playlist.  This is critical to merging signal-only samples into a full MC sample.
  const double totalDataPOT = std::accumulate(playlistToDataPOT.begin(), playlistToDataPOT.end(), 0., [](double sum, const auto& pair) { return sum + pair.second; });

  outFile->cd();
  for(auto entry: mergedSamples)
  {
    if(scaleToDataPOT)
    {
      std::cout << "Scaling by " << totalMCPOT << " / " << totalDataPOT << " = " << totalMCPOT / totalDataPOT << "\n";
      if(dynamic_cast<PlotUtils::MnvH1D*>(entry.second))
      {
        static_cast<PlotUtils::MnvH1D*>(entry.second)->Scale(totalMCPOT / totalDataPOT);
      }
      else
      {
        assert(dynamic_cast<PlotUtils::MnvH2D*>(entry.second));
        static_cast<PlotUtils::MnvH2D*>(entry.second)->Scale(totalMCPOT / totalDataPOT);
      }
    }
    entry.second->Write();  
  }
  for(auto entry: mergedPOT) entry.second->Write();
  for(auto entry: mergedNucleons) entry.second->Write();
  for(auto entry: mergedMetadata) entry.second->Write();
  for(auto entry: mergedFlux)
  {
    if(dynamic_cast<PlotUtils::MnvH1D*>(entry.second))
    {
      static_cast<PlotUtils::MnvH1D*>(entry.second)->Scale(1./totalDataPOT);
    }
    else
    {
      assert(dynamic_cast<PlotUtils::MnvH2D*>(entry.second));
      static_cast<PlotUtils::MnvH2D*>(entry.second)->Scale(1./totalDataPOT);
    }
    entry.second->Write();
  }

  return success;
}
