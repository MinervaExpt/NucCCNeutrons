//File: CmdLine.h
//Brief: CmdLine encompasses all of the options ProcessNeutronMC/Data
//       derive from the command line.  Its constructor standardizes how
//       I construct the objects I need, and it exposes as members all of
//       the resources constructed.
//
//       Its resources are only valid as long as the CmdLine object itself
//       is valid.  It throws CmdLine::exceptions on failure to read
//       command line arguments correctly.
//
//       I chose to read arguments from the command line to take advantage
//       of utilities that shells already provide for me.  Why try to
//       implement globbing behavior when the shell already does it
//       for me more effectively than I ever could and is maintained
//       by someone else?  I'm following GTK's philosophy of only taking
//       files as command line arguments because I've found complex
//       command line interfaces very onerous to reproduce when controlling
//       physics analysis code.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APP_CMDLINE_H
#define APP_CMDLINE_H

//c++ includes
#include <string>
#include <memory>
#include <vector>

class TFile;

namespace YAML
{
  class Node;
}

namespace app
{
  class CmdLine
  {
    public:
      CmdLine(const int argc, const char** argv);
      ~CmdLine(); //A great opportunity to make sure my histograms are always saved.
  
      std::unique_ptr<TFile> HistFile; //File where histograms will be written

      //const access to names of NTuple files to read
      inline const std::vector<std::string>& TupleFileNames() const { return fTupleFileNames; }

      //const access to the parsed configuration file for this job
      inline const YAML::Node& ConfigFile() const { return *fConfigFile; }

      inline bool isMC() const { return fIsMC; }

      inline const std::string& playlist() const { return fPlaylist; }

      //Error codes that I can return to the operating system.  Might be useful
      //if applications using CmdLine are ever part of bash scripts.
      enum ExitCode: int
      {
        Success = 0, //Nothing wrong
        BadCommandLine, //The command line couldn't be parsed
        BadOutputFile, //Couldn't create the file for output histograms
        YAMLError, //Failed to parse the YAML file
        AnalysisError,
        GeometryError,
        MixedMCAndData,
        IOError
      };

      //Catching this type of exception indicates that something has gone
      //wrong reading options from the command line.
      class exception: public std::runtime_error
      {
        public:
          exception(const std::string& appName, const std::string& what, const ExitCode why);
          virtual ~exception() = default;

          const ExitCode reason;
      };

    private:
      std::unique_ptr<YAML::Node> fConfigFile; //The parsed configuration file for this job.
                                               //Using a const accessor function is easier than creating it const.
      std::vector<std::string> fTupleFileNames; //List of NTuple files to read

      bool fIsMC; //Am I processing data or MC files?  Decided based on the
                  //first file to be processed.  Output file gets named based
                  //on this, and I also make sure the two don't get mixed,
                  //even in OPTimized builds.

      std::string fPlaylist; //Playlist name for FluxReweighter

      //Centralize argument parsing that is shared between argv and STDIN
      void HandleArg(const std::string& scriptName, const std::string arg, std::string& outFileName, std::string& configFile);
  };
}

#endif //APP_CMDLINE_H
