//File: CmdLine.cpp
//Brief: CmdLine encompasses all of the options ProcessNeutronMC/Data
//       derive from the command line.  Its constructor standardizes how
//       I construct the objects I need, and it exposes as members all of
//       the resources constructed.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Usage:
#define USAGE " <cuts.yaml> [configs.yaml]... <tuple.root> [tuples.root]..."\
"\n\nOR with #!/usr/bin/env ProcessAnaTuples at the top of <cuts.yaml>:\n\n"\
"<cuts.yaml> [configs.yaml]... <tuple.root> [tuples.root]...\n\n"\
"\tThis application also accepts configuration and tuple\n"\
"\tfiles via STDIN, so you can also:\n\n"\
"\tfind /pnfs/minerva/persistent/users/$USER/yourProcessing -name *.root\n"\
"\t| <cuts.yaml> [moreConfigsInOrder.yaml]...\n\n"\
"\t<cuts.yaml> [moreConfigsInOrder.yaml]... < playlist.txt\n\n"\
"\tAccepts filenames and regular expressions that are xrootd\n"\
"\tURLs just like regular file names.  Of course, the shell\n"\
"\tcan't expand those, so I try to do it for you.\n"\
"\tYAML configuration files are first searched for in the\n"\
"\tcurrent directory and/or as absolute paths, and then they\n"\
"\tare looked for in:\n" \
INSTALL_DIR "bin.\n"
//TODO: Maybe point to documentation in installation directory?  I'd have to
//      get CMake to tell me what the installation directory is.

//NucCCNeutrons includes
#include "app/CmdLine.h"
#include "app/IsMC.h"
#include "app/GetPlaylist.h"

//PlotUtils includes
#include "PlotUtils/ROOTglob.h"
#include "PlotUtils/ErrorHandler.h"

//YAML-cpp includes
#include "yaml-cpp/yaml.h"

//ROOT includes
#include "TFile.h"
#include "TSystem.h"

//c++ includes
#include <fstream>
#include <iostream>

//C UNIX include (POSIX?)
#include <unistd.h>
#include <stdio.h>

namespace app
{
  void CmdLine::HandleArg(const std::string& binName, const std::string arg, std::string& outFileName, std::string& configFile)
  {
    if(arg.find(".root") != std::string::npos)
    {
      //The shell can't expand regular expressions in xrootd URLs,
      //so use a PlotUtils function to handle them for me.
      if(arg.find("root:") != std::string::npos)
      {
        const auto found = PlotUtils::glob(arg, *gSystem);
        fTupleFileNames.insert(fTupleFileNames.end(), found.begin(), found.end());
      }
      else fTupleFileNames.push_back(arg);
    }
    else if(arg.find(".yaml") != std::string::npos)
    {
      //First, handle absolute paths and files in the current directory
      std::ifstream readFile;
      readFile.open(arg);
      if(!readFile.is_open())
      {
        readFile.open(std::string(INSTALL_DIR "bin/") + arg);
        if(!readFile.is_open()) throw exception(binName, std::string("No such file or directory: ") + arg + ".\n", ExitCode::BadCommandLine);
      }

      //Eventually set the name of the output file to the last YAML file passed.
      //The last file is the one that has to contain the keywords.
      outFileName = arg;
 
      configFile.append(std::istreambuf_iterator<char>(readFile), std::istreambuf_iterator<char>());
    }
    else
    {
      throw exception(binName, std::string("Got command line argument that is neither a ROOT file nor a YAML file: ") + arg + ".\n", ExitCode::BadCommandLine);
    }
  }

  CmdLine::CmdLine(const int argc, const char** argv)
  {
    std::string configFile;
    const std::string binName = argv[0];
    std::string outFileName = binName; //I will override this with the first
                                       //YAML file I get on the command line.

    //First, read from STDIN.
    //This lets me do things like make ProcessNeutronMC look like an
    //interpreter to the shell.
    if(!isatty(fileno(stdin)))
    {
      std::string line;
      while(std::getline(std::cin, line))
      {
        #ifndef NDEBUG
          //std::cout << "Reading " << line << " from STDIN...\n";
        #endif
        HandleArg(binName, line, outFileName, configFile);
      }
    }

    for(auto whichArg = 1; whichArg < argc; ++whichArg) //Skip the first argument because it's always the executable that started this program
    {
      #ifndef NDEBUG
        //std::cout << "Reading argument " << argv[whichArg] << " from the command line...\n";
      #endif
      HandleArg(binName, argv[whichArg], outFileName, configFile);
    }

    if(fTupleFileNames.empty())
    {
      throw exception(binName, "No *.root AnaTuple files found on the command line.\n", ExitCode::BadCommandLine);
    }

    fIsMC = app::IsMC(fTupleFileNames.front());
    fPlaylist = app::GetPlaylist(fTupleFileNames.front(), fIsMC); //TODO: Generalize hard-coded AnaTuple name
    #ifndef NDEBUG
      //std::cout << "Am I processing MC?  " << std::boolalpha << fIsMC << "\n";
    #endif

    if(configFile.empty())
    {
      throw exception(binName, "No *.yaml configuration files found on the command line.\n", ExitCode::BadCommandLine);
    }

    //Try to create a TFile to write the produced histograms.  Quit without
    //doing anything if I fail.
    outFileName.erase(outFileName.find("."));
    outFileName.erase(0, outFileName.rfind("/") + 1);
    outFileName += std::string((fIsMC?"MC":"Data")) + ".root";

    try
    {
      HistFile.reset(TFile::Open(outFileName.c_str(), "CREATE"));
    }
    catch(const ROOT::exception& e)
    {
      //Print some extra information if I failed to create the output file.
      throw exception(binName, "Couldn't create a TFile named " + outFileName + " in the current directory.  If it already exists, I refuse to overwrite it!\n", ExitCode::BadOutputFile);
    }
    
    //Write the assembled configuration file to a file in the current
    //directory.  Makes sure I know exactly what parameters I ran with.
    {
      std::ofstream configCopy("ReRun" + outFileName.substr(0, outFileName.find(".")) + ".yaml");
      configCopy << configFile;
    }

    try
    {
      fConfigFile.reset(new YAML::Node(YAML::Load(configFile)));
    }
    catch(const YAML::Exception& e)
    {
      throw exception(binName, std::string("YAML error from yaml-cpp:\n") + e.what() + "\n", ExitCode::YAMLError);
    }
  }

  CmdLine::~CmdLine()
  {
    HistFile->Write(); //Ensure that all histograms written to HistFile
                       //get saved to the filesystem just before the job
                       //ends.
  }

  CmdLine::exception::exception(const std::string& appName, const std::string& what,
                                const CmdLine::ExitCode why): std::runtime_error((what + "Usage:\n" + appName + USAGE).c_str()),
                                reason(why)
  {
  }
}
