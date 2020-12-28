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
2. `cd NucCCNeutrons`
3. `git clone https://github.com/MinervaExpt/NucCCNeutrons.git src`
4. Install dependencies listed below.  Help CMake find them by setting e.g. `PlotUtils_DIR=/home/aolivier/app/ThesisAnalysis/PlotUtils/opt`
5. ``cd ../debug && mkdir build && cd build && cmake ../../src -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Debug "-Dyaml-cpp_DIR=/path/to/yaml-cpp/lib/cmake/yaml-cpp" #You might need to specify paths to dependencies too``
6. `make install #Can parallelize with e.g. make install -j 8`

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

### Command Line
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

### Physics Analysis Structure
A complete YAML file for ProcessAnaTuples should have a map/list with each of these names:
1. `systematics`: **List** of alternative universes in which some analysis information is shifted.  Their RMS in each histogram will become the systematic uncertainty on that histogram's bin contents.
2. `model`: Weights applied to events to modify the underlying neutrino interaction model simulated by [GENIE](http://www.genie-mc.org/).
3. `signal`: A Study fills histograms under 4 different conditions.  These conditions can be combined to make all of the histograms needed to extract a differential cross section.  There can only be one Study.
3. `fiducials`: Fiducial volumes in which to perform the `signal` Study.  The `signal` Study is performed seprately in each fiducial volume.
4. cuts: Define the phase space in which the `signl` Study will be performed.  `truth` cuts are really SignalConstraints.  `phaseSpace` constraints on the signal can be corrected for in a cross section as part of acceptance.  Events that fail the `signal` constraints themselves are backgrounds that must be subtracted from a measured event rate.  `reco` cuts seek to emulate the `truth` signal definition as much as possible, but will ultimately make mistakes.
5. `sidebands`: Alternative phase space regions that help constrain `backgrounds` based on data.  Ideally, a sideband defines a similar phase space to the `reco` `cuts`, but it is dominated by one of the `backgrounds`.  A sideband only makes sense if it requires that an event `fails` some of the cut names from `cuts`.  It may also require that an event `passes` additional cuts.  It's a Study just like the `signal`.
6. `backgrounds`: Events that fail the `truth` `cuts` can be further broken down.  Individual `backgrounds` may be fit individually among multiple `sidebands` to model the interplay between different physics processes.
7. `app`: Extra information that the systematics framework needs to do its job.  Right now, this just means `nFluxUniverses` and `useNuEConstraint`.  Maybe I should call it `flux` instead. 

### File Format
Most Studies supported by ProcessAnaTuples produce .root files that contain:
- `TNamed` NucCCNeutronsGitCommitHash: Commit hash with which ProcessAnaTuples was built before it was run.  This may be out of date if you compile ProcessAnaTuples with uncommitted changes!  If you are disciplined with making commits before producing major results, this hash combined with the output .yaml file from ProcessAnaTuples lets you reproduce the job that made a .root file.  Remember that UnfoldUtils and PlotUtils commits are not (yet) recorded.
- `TParameter<double>` POTUsed: Protons On Target used to produce a .root file.  Useful for comparing data to Monte Carlo samples with a different simulated exposure.  Counted for each input AnaTuple that can be opened.
- `TParameter<double>` `<Fiducial>_FiducialNucleons`: Number of nucleons in each entry in the `fiducials` map.  Needed to extract a cross section.
- `PlotUtils::MnvH1D` and `PlotUtils::MnvH2D`: Histograms like TH1D, but with 1 extra histogram for each systematic universe.  They can report a systematic uncertainty in each bin by taking the RMS of all universes in that bin.  Each universe's histogram is a MnvVertErrorBand.  Read about MINERvA's PlotUtils product to learn about what MnvH1D can do.

Studies can also produce TTrees and text files.

`<Study>_EAvailableBackgroundData<Fiducial>.md` is a markdown file that summarizes how each of the `reco` `cuts` performed on the sample processed.  You can make a nice PDF table with `pandoc -o file.pdf file.md`.  Pandoc may also support cross-compiling into LaTeX.

### TODO: Anatomy of a Study

### TODO: How to write a Cut
