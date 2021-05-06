//File: Sideband.h
//Brief: A Sideband keeps track of all of the Backgrounds in a single sideband for a single systematic Universe.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef FIT_SIDEBAND_H
#define FIT_SIDEBAND_H

//ROOT includes
#include "TDirectoryFile.h"
#include "TH1D.h"

//c++ includes
#include <vector>
#include <string>
#include <memory>

namespace fit
{
  class Sideband
  {
    public:
      //Construct a Sideband in the CV
      Sideband(const std::string& sidebandName, TDirectoryFile& dataDir, TDirectoryFile& mcDir, const std::vector<std::string>& floatingBackgroundNames, const std::vector<std::string>& fixedNames, const bool floatSignal = false);

      //Construct a Sideband in a specific, non-CV, systematic universe using the CV sideband as the source of histograms.
      //This makes sure the sideband histograms' modifications are written to the output file with the CV.
      Sideband(const Sideband& cvSideband, const std::string& errorBandName, const int whichUniv);

      //Observer pointers to histograms that came from (and will be deleted by) a TFile
      TH1D data;
      std::unique_ptr<TH1> fixedSum;
      std::vector<TH1D*> floatingHists;

      //Make the compiler happy.
      Sideband(const Sideband& parent);
  };
}

#endif //FIT_SIDEBAND_H
