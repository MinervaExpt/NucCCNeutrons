# NucCCNeutrons

## Description
Produces a cross section in neutron multiplicity from MINERvA data.  This is just the macro component that consumes AnaTuples from a Gaudi AnaTool of the same name.  Andrew Olivier's thesis project.  This version relies heavily on Ben Messerly's New Systematics Framework.

My goal is to build on the machinery used to produce [MINERvA's low energy neutron paper](https://arxiv.org/abs/1901.04892) and [Miranda Elkins' masters thesis](http://inspirehep.net/record/1609603?ln=en) to:
1. Study neutrons from neutrino interactions as a function of nucleus using [MINERvA's](https://arxiv.org/abs/1305.5199) nuclear targets
2. Extract a differential cross section in neutron multiplicity as a function of nucleus

## Installation
1. Make a directory structure for the repository to live in.  I recommend an out of source build in NucCCNeutrons/:
  * src
  * debug
  * opt
2. cd NucCCNeutrons
3. git clone https://github.com/MinervaExpt/NucCCNeutrons.git src
4. Install dependencies listed below.  Help CMake find them by setting e.g. `PlotUtils_DIR=/home/aolivier/app/ThesisAnalysis/PlotUtils/opt`
5. `cd ../debug && mkdir build && cd build && cmake ../../src -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Debug "-Dyaml-cpp_DIR=/path/to/yaml-cpp/lib/cmake/yaml-cpp" #You might need to specify paths to dependencies too`
6. make install #Can parallelize with e.g. make install -j 8

## Dependencies
1. [PlotUtils](https://cdcvs.fnal.gov/redmine/projects/minerva-sw/repository/show/AnalysisFramework/Ana/PlotUtils)
2. [UnfoldUtils](https://cdcvs.fnal.gov/redmine/projects/minerva-sw/repository/show/AnalysisFramework/Ana/UnfoldUtils)
3. [ROOT 6](https://root.cern.ch/building-root)
4. [YAML-cpp 0.6.0](https://github.com/jbeder/yaml-cpp)
5. [BaseUnits](https://github.com/aolivier23/BaseUnits) **Now embedded**

## Usage
Set up the libraries on Linux systems:
1. `source NucCCNeutrons/debug/bin/setup.sh #debug -> opt for faster optimized binary if you built it`
2. `source root/opt/bin/thisroot.sh #Make ROOT libraries visible on LD_LIBRARY_PATH`
3. Put `libPlotUtils.so` on `LD_LIBRARY_PATH`.  With my CMake build, I do `source PlotUtils/debug/bin/setup.sh`.  With the traditional cmt build, I do `source Ana/PlotUtils/PlotUtils/cmt/setup.sh`.  The cmt build probably has different capitalization which I should fix.
4. Put `libUnfoldUtils.so` on `LD_LIBRARY_PATH`.  Should be the same procedure as PlotUtils above.

This makes the shell default to finding the executables, libraries, and files it needs.

All analysis programs are run by the `ProcessAnaTuples` program.  At the highest level, an analysis program is described by a YAML file in the `config` directory.  Under the hood, ProcessAnaTuples combines all YAML files on the command line into a single program.  So, I could, and do, write an intentionally incomplete .yaml file like NeutronMultiplicityTracker.yaml to combine with configuration "flags" on the command line: `ProcessAnaTuples CVOnly.yaml MnvGENIEv1.yaml Tracker.yaml NeutronMultiplicity.yaml <some .root files>`.

###Command Line
Inputs to ProcessAnaTuples:
- 1 or more .yaml files to decide what Cuts, histogramming code, and models are used
- 1 or more .root files with AnaTuples to process.  At the moment, they cannot be a mixture of data and MC.
- Any and all of these can come from a pipe or file redirection via the UNIX stdin file descriptor.  Think about `find /pnfs/minerva/persistent/users/aolivier -name "*.root" | xrdify | ProcessAnaTuples NeutronMultiplicity.yaml`
- No command line flags ever!  Really!  OK, `-h` and `--help` do useful things because I'm not evil.

Outputs from ProcessAnaTuples:
- `<name of last .yaml file><MC|Data>.root`: histograms produced with embedded POT and version information
- `<name of last .yaml file><MC|Data>.md`: "Cut table" with a summary of run conditions.  Ready for `pandoc` to convert to a PDF.
- Note: Don't pipe the output of ProcessAnaTuples to anything right now because it's a mess.  Making stdout useful again is a TODO.
- Help information on stderr

###Physics Analysis Structure
TODO

###Systematics Format: MnvH1D
TODO
