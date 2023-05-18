# NucCCNeutrons

## Description
Produces a cross section for neutron production in muon transverse momentum from MINERvA data.  This is the macro component that consumes AnaTuples from a Gaudi AnaTool of the same name.  Andrew Olivier's thesis project.  This version relies heavily on Ben Messerly's New Systematics Framework.

My goal is to build on the machinery used to produce [MINERvA's low energy neutron paper](https://arxiv.org/abs/1901.04892) and [Miranda Elkins' masters thesis](http://inspirehep.net/record/1609603?ln=en) to:
1. Study neutrons from neutrino interactions as a function of nucleus using [MINERvA's](https://arxiv.org/abs/1305.5199) nuclear targets
2. Extract a differential cross section for multiple neutrons to be produced as a function of muon transverse momentum

## Installation
TODO: Update this
Old Manual install instructions:
1. Make a directory structure for the repository to live in.  I recommend an out of source build in NucCCNeutrons/:
  * NucCCNeutrons
  * debug
  * opt
2. `cd NucCCNeutrons`
3. `git clone https://github.com/MinervaExpt/NucCCNeutrons.git src`
4. Install dependencies listed below.  Help CMake find them by setting e.g. `PlotUtils_DIR=/home/aolivier/app/ThesisAnalysis/PlotUtils/opt`
5. ``cd ../debug && mkdir build && cd build && cmake ../../src -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Debug "-Dyaml-cpp_DIR=/path/to/yaml-cpp/lib/cmake/yaml-cpp" #You might need to specify paths to dependencies too``
6. `make install #Can parallelize with e.g. make install -j 8`

## Dependencies
1. [ROOT 6](https://root.cern.ch/building-root)
1. [MAT-MINERvA](https://github.com/MinervaExpt/MAT-MINERvA)
2. [UnfoldUtils](https://github.com/MinervaExpt/UnfoldUtils)
4. [YAML-cpp 0.6.0](https://github.com/jbeder/yaml-cpp)
5. See also GENIEXSecExtract for warping studies: [GENIEXSecExtract](https://github.com/MinervaExpt/GENIEXSecExtract)

## Usage
Set up the libraries on Linux systems:
1. `source NucCCNeutrons/opt/bin/setup.sh #This really sets up MAT-MINERvA, but it's in the same "install prefix" as this package if you follow my instructions
2. `source root/opt/bin/thisroot.sh #Make ROOT libraries visible on LD_LIBRARY_PATH`

This makes the shell default to finding the executables, libraries, and files it needs.

All analysis programs are run by the `ProcessAnaTuples` program.  At the highest level, an analysis program is described by a YAML file in the `config` directory.  Under the hood, ProcessAnaTuples combines all YAML files on the command line into a single program.  So, I could, and do, write an intentionally incomplete .yaml file like NeutronMultiplicityTracker.yaml to combine with configuration "flags" on the command line: `ProcessAnaTuples CVOnly.yaml MnvGENIEv1.yaml Tracker.yaml NeutronMultiplicity.yaml <some .root files>`.

### Command Line
Inputs to ProcessAnaTuples:
- 1 or more .yaml files to decide what Cuts, histogramming code, and models are used
- 1 or more .root files with AnaTuples to process.  They cannot be a mixture of data and MC.
- Any and all of these can come from a pipe or file redirection via the UNIX stdin file descriptor.  Think about `find /pnfs/minerva/persistent/users/aolivier -name "*.root" | xrdify | ProcessAnaTuples NeutronMultiplicity.yaml`
- No command line flags ever!  Really!  `-h` and `--help` do useful things because I'm not evil.

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

## Important Workflows

### Extracting a Cross Section
A cross section is the "money plot" in just about any MINERvA paper.  This is a summary of how to reproduce Figure TODO in [my thesis]().  I'll update it later with tested instructions.

1. Set up this package and get the input files
  1. Install this package and its dependencies.
  2. You should already know the bin low edges you want to use.  You can study different choices using warping studies which are described below.
  3. You need access to my "AnaTuples", .root files with a single TTree that came from the NucCCNeutrons "AnaTool" in the MINERvA offline framework.  Contact the author, or failing that the MINERvA collaboration, for access to these.  They take about 1TB of storage.  My analysis doesn't currently work with data preservation AnaTuples because their neutron information is in a different format.  The information may well all be there in which case a branch to adapt ProcessAnaTuples to work with MasterAnaDev would be most welcome!
2. Produce cross section histograms for each "playlist".  A playlist is a MINERvA data run period during which detector and beam conditions didn't change much.
  1. Make a new working directory ;)  If you're on the MINERvA GPVMs, put it in /pnfs/minerva or /minerva/data if you have to.  **`cd` to that directory.**
  2. Make a copy of `config/multiNeutron_MnvTunev1.yaml`.  Change the binning on line 175 for example.
  3. Make a data histogram file for each playlist.  On your own laptop, `scripts/runningLocally/forEachPlaylistData.sh` might help.  **Don't do this on the GPVMs** because their disks are slow and they don't support many threads.  If you have to run at Fermilab, run on the FermiGrid instead using scripts/TODO.
  4. The data histograms should just take a few minutes interactively.  If they all finished without error messages, then you're ready to move on to MC histograms.
  5. Make an MC histogram file for each playlist.  On your own laptop, `scripts/runningLocally/forEachPlaylist.sh` might help.  **Don't do this on the GPVMs** bec
ause their disks are slow and they don't support many threads.  If you have to run at Fermilab, run on the FermiGrid instead using scripts/TODO.
  6. Make an MC histogram file for the "special cross-talk shifted sample".  Run `multiNeutron_noSysts.yaml` over TODO.  This becomes its own systematic uncertainty.  It should be as fast as processing the data because you don't need to calculate systematic uncertainties on this sample.
  7. If you're going to make cross section comparison plots like the conclusion of my thesis, you'll need to analyze the "Valencia" and "SuSA" models too, at least in MC.  Do the same thing you just did for MnvTunev1 using `multiNeutron_Valencia.yaml`.  SuSA is covered in the next step.
  8. If you want to compare to SuSA or simulate 2p2h from 1.2 GeV/c to 2.0 GeV/c in q3 (both are in my paper), then you'll need to make _even more_ MC files.  Run `config/multiNeutron_2p2hOff.yaml` over the standard tuples and `config/multiNeutron_MnvTunev1.yaml` over the "special 2p2h sample" AnaTuples.
3. Combine playlists
  1. Make a "merged" directory to work in.  After MC histograms, you'll have a bunch of directories with names like "me6A".  "merged" should be in the same directory as these.
  2. Merge the data with `MergeAndScaleByPOT me*/multiNeutron_MnvTunev1Data.root`.  Put the result in `merged`
  3. For MnvTunev1 itself, without the extended 2p2h sample:
    1. `cd merged`
    2. Merge the MC with `MergeAndScaleByPOT ../me*/multiNeutron_MnvTunev1MC.root`
    3. Do the same for the Valencia model
  4. For the extended 2p2h sample, things are more complicated because we didn't simulate MINERvA's last 3 playlists.  They have roughly the same flux as minervame6G, so we make one "super playlist" that then gets merged with the other playlists.  I wrote a script to handle this because it's easy to get wrong.
    1. Merge the MC with `scripts/runningLocally/mergeWithSpecial2p2h.sh`
    2. You might need to edit this script and do this again for the SuSA sample
4. Add special systematic uncertainties:
  1. You ran over the "special cross-talk shift sample" earlier, right?  You made a SuSA cross section too, right?  If not, do this now.  SuSA may take a few hours.
  2. Copy everything in `scripts/runningLocally` into `merged` and `cd merged` if you're not already there.  You should have all of the final merged files in this directory.
  3. Run `preprocessForCrossSection.sh`.  It should work on just the MnvTunev1 files by default.
5. Constrain backgrounds using data
  1. Each fitting stage looks like this: `FitSidebands someConfig.py multiNeutron_MnvTunev1Data_merged.root multiNeutron_MnvTunev1MC_merged.root`.  It produces a `multiNeutron_MnvTunev1MC_merged_constrainedBy_someConfig.root` which is the result.  It's interesting to compare the fit result to the pre-fit result in each sideband as descibed in [Other Studies in my Thesis].  The uncertainties should become much smaller as you constrain the background.
  2. Run over 4 fit stages in order:
     - oneEAvailBackgroundFit.yaml
     - lowpTLinearFit.yaml
     - highpTLinearFit.yaml
     - overflowBinFit.yaml
5. Make cross section plots
  1. `ExtractCrossSection multiNeutron_MnvTunev1Data_merged.root multiNeutron_MnvTunev1MC_merged_TODO.root` produces another .root file with histograms in it named `crossSection`.  These are cross sections **extracted from the data** combined with our MC!  This is the reason you're here!  This program is basically Formula TODO from my thesis.  **Rename it now:** `mv Tracker_crossSection.root crossSection_constrained.root`.
  2. Extract a cross section _prediction_ from the MC to compare to: `ExtractCrossSection multiNeutron_MnvTunev1MC_merged_TODO.root multiNeutron_MnvTunev1MC_merged_TODO.root`.  This is saying, "use the MC as if it were data too".  This is the "MnvTunev1 cross section prediction" in this example.  To use my scripts out of the box, you'll need to **do the same for SuSA and Valencia** 2p2h models.  As you make each file, **rename it**: `mv Tracker_crossSection.root crossSection_MnvTunev1.root`
  3. Run `python compareCrossSection_singlePane.py` in your current directory.  It will automatically look for files named `crossSection_constrained.root`, `crossSection_MnvTunev1.root`, `crossSection_SuSA.root`, and `crossSection_Valencia.root`.  It will produce `crossSectionComp.png`, `uncertaintySummary.png`, and `chi2Table.md` which should match my thesis and my soon-to-be-published PRD paper!

### TODO: Other Studies in my Thesis
1. MC Breakdown
2. Warping Studies
3. Background Breakdowns before and after fitting
4. Miranda's Neutron Counting Plots
5. Verify Against CCQENu/MasterAnaDev

## Adding to ProcessAnaTuples
`ProcessAnaTuples` is designed to make exactly the plots you need to extract a differential cross section in 1D.  It can also be configured to produce plots needed to decide how to extract a cross section like deciding how to bin the cross section, deciding how many iterations of unfolding to use, and checking that the physics extracted is compatible with Miranda's neutron paper.  Each of these use cases is implemented by a `Study`.  The cross section you extract is defined by the `Cut`s you apply to the data and the MC.  It's interesting to extract cross sections assuming different models in the MC, and each model is implemented as a series of `Reweighter`s.  You might also want to extract the same cross section for different sections of solid material in the MINERvA detector called `Fiducial`s.  But most of the useful fiducial regions in the MINERvA detector where this analysis will work have already been implemented for you.  So, the follow section explains how to add a `Study`, a `Cut`, and a `Reweighter` to ProcessAnaTuples.

### How to Write a Study
A `Study` is a group of plots produced from data and/or MC.  All Studies in NucCCNeutrons are in the `analyses/studies` directory.  There are two kinds of files in this directory:
- variables which are used to extract a differential cross section and perform other studies
- stand-alone Studies which make plots we need that don't fit into the model of what's strictly needed to extract a 1D differential cross section

A "variable" implements a physics observable that we can unsmear using one of RooUnfold's unfolding techniques.  The code for a "variable" plugs into the Studies `CrossSectionSignal` and `CrossSectionSideband`.  The advantage of this design is that `CrossSectionSignal` and `CrossSectionSideband` make it very easy to extract a cross section without making mistakes and with a minimum of extra code.  The disadvantage is that you have to understand class templates if you really want to understand how they work.  Instead, I'll teach you that there are 4 functions that a new "variable" must implement to work:
- `reco(const evt::Universe& event) const`: Returns a reconstructed quantity using only functions of `evt::Universe` that are available to data files.  These same functions should be available to MC files as well per the design of the NucCCNeutrons "AnaTool"
- `truth(const evt::Universe& event) const`: Returns a simulated quantity using only functions of `evt::Universe` available to the "truth tree" in MC files.  There are also "MC tree" functions, but they will not work for calcuating the "efficiency denominator".
- `inline std::string name() const`: returns the name of this variable that will be used in histogram names
- A constructor like `MuonPT(const YAML::Node& config)`: This is empty for most "variables" so far.  If your variable needs to do complicated things like access a file, you can pass any YAML arguments you want to this function and save them into your class structure here.

To make a new variable visible to ProcessAnatuples, you need to do two things:
- Register its cross section Studies at the end of its .cpp file with:
```
namespace
{
  static ana::CrossSectionSignal<ana::MuonMomentum>::Registrar MuonMomentumSignal_reg("MuonMomentumSignal");
  static ana::CrossSectionSideband<ana::MuonMomentum>::Registrar MuonMomentumSideband_reg("MuonMomentumSideband");
}
```
  Replace `ana::MuonMomentum` with the name of your new variable class.
- Add its .cpp file to the library in `analyses/studies/CMakeLists.txt`

A "stand-alone Study" gives you maximum flexibility for making plots at the cost of greatly increased complexity.  You'll need to understand how to write a c++ class to "implement an interface" to do this.  The interface you're implementing is `analyses/base/Study.h`.  It exposes 4 "event loop functions" where you do most of your physics and a few "once per job functions" to tell ProcessAnaTuples how to optimize for your Study.  3 of the "event loop functions" have "twins" like this:
- `virtual void truth(const evt::Universe& event, const events weight)`
- `virtual void mcSignal(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt)`

Use the first version when you're new to this package.  It's a lot simpler to work with!  The second version is an "expert interface" to make specific Studies much faster.  If you really need the expert interface, you should probably be talking to the original author of this package!

To make a new Study visible to ProcessAnaTuples, you need to do two things:
- Register it at the end of its .cpp file with:
```
namespace
{
  static ana::Study::Registrar<ana::NeutronDetection> NeutronDetection_reg("NeutronDetection");
}
```
  Replace "NeutronDetection" with the class name of your new Study.
- Add its .cpp file to the library in `analyses/studies/CMakeLists.txt`

Let's talk about the physics you do in each of the 4 event loop functions.  Each function is an opportunity to `Fill()` histograms or print out information about events that meet certain criteria relevant to extracting a cross section:
- `virtual void data(const evt::Universe& event, const events weight = 1_events)`: Access reconstruction information for an event that passed all reconstruction cuts.  This event would have made it into the raw data points that go on your cross section plot.  We don't know, and can't find out at this point, whether it satisfies the signal definition.  This is called for both data and MC.  In the MC, these events are "fake data" that can be useful for the closure test.
- `virtual void mcSignal(const evt::Universe& event, const events weight)`: Access reconstruction and simulation information for an event that passed all reconstruction cuts **and** satisfies the signal definition.  These are the events that go into the efficiency numerator and the migration matrix for extracting a cross section.  You can match reconstruction information with what was simulated in this function.  Always fill all histograms with the `weight` provided for the "model" block to work correctly.
- `virtual void mcBackground(const evt::Universe& event, const background_t& background, const events weight)`: Probably the least useful function for custom studies.  Access reconstruction and simulation information for an event that passed all reconstruction cuts **and does not** satisfy the signal definition.  The "background" object has a `name()` and can be mapped to a histogram in a `util::Categorized<>`.  The background was chosen using only true simulated information.  This lets you make a separate histogram for each background category.  Per-background histograms are used for the sideband fit.
- `virtual void truth(const evt::Universe& event, const events weight)`: Access only simluation information for an event that passed the signal definition.  In this function, you don't know whether this event satisfies the reconstruction cuts.  For a cross section, this is where the efficiency denominator is filled.  You must not try to access reconstruction information here.  You might use this function to plot what GENIE processes end up satisfying your signal definition.

Histograms should be set up in your Study's constructor.  For the `CrossSectionSignal` class in `analysis/studies/CrossSectionSignal.h` for example, this is the function named `CrossSectionSignal(const YAML::Node& /*config*/, util::Directory& /*dir*/, cuts_t&& mustPass, const std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::Universe*>>& /*universes*/))`.  Use the `config` argument if you want your Study to have cuts you can change from the .yaml file or read files.  Leave the other arguments along and pass them to the base class constructor like `CrossSectionSignal` does.

The MAT replaces `TH1D` with systematics-aware histograms like `HistWrapper<evt::Universe>`.  ProcessAnaTuples takes this one step further with units-aware HistWrappers: `units::WithUnits<HistWrapper<evt::Universe>, UNIT, events>`.  Basically, use them like `analyses/studies/NeutronDetection.cpp` does on line 134: `fCandsPerFSNeutron->Fill(&event, neutrons(candsPerFS.count(withCands)), weight)`.  `fCandsPerFSNeutron` is a member variable that's a pointer to a `units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, neutrons, events>`.  It's created using the `util::Directory::make<>()` interface on line 68.  Making it a member variable makes it available to be set up only once in the constructor and then filled in any of the event loop functions.  Using the `util::Directory::make<>()` interface makes sure it gets put in the right file when ProcessAnaTuples is finished.  Its arguments are a systematic universe, `&event`, the number of neutrons to plot, and a weight. The systematic universe is the MAT's convenient way to make the same plot under many different hypotheses about what might be different about our detector.  The number of neutrons itself is a `quantity<>` with units.  If you made a `util::WithUnits<HistWrapper<evt::Universe>, GeV, events>` for example, the program would know to automatically convert numbers it's Fill()ed with into GeV to match the labels on the x axis regardless of whether they're MeV, GeV, or something else.  You almost always want to pass the event's weight to any `Fill()` call so that the model used to make your histograms matches what's described in the .yaml file.

There are two other functions that advanced users might want to use:
- `virtual bool wantsTruthLoop() const`: This should be a one-line function: `return false`.  If your `truth()` function doesn't do anything, you can make `ProcessAnaTuples` a little faster by overriding this function.  Returning false here prevents `ProcessAnaTuples` from looping over the `Truth` tree which is usually fairly expensive.  You probably don't need this level of optimization.
- `virtual void afterAllFiles(const events /*passedSelection*/)`: Gets called at the very end of the event loop.  If you _really_ need to divide a histogram by the number of entries processed, you can do that here.  This should almost never be needed.

### How to Write a Cut
A cut defines a kinematic region in which a cross section will be extracted.  ProcessAnaTuples uses two kinds of Cuts: reco::Cut and truth::Cut.  A reco::Cut uses only reconstructed information.  A truth::Cut is another name for `PlotUtils::SignalConstraint<evt::CVUniverse>`.  It uses only simulation information to identify which simulated events match the criteria for the cross section you're extracting.  In short, every `reco::Cut` makes an experimentalist's corrections more believable, but theorists who want to use your result need to know your `truth::Cut`s.

Both `reco::Cut` and `truth::Cut` have only 1 function that you need to implement.  The `reco::Cut` version looks like this: `virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override`.  The `truth::Cut` version is just missing the "empty Event" structure: `virtual bool passesCut(const evt::Universe& event) const override`.  It's helpful to be able to change where a cut is applied without re-compiling c++ code.  The `reco::Cut` and `truth::Cut` constructors take a `YAML::Node` that gives you access to part of the YAML file that controls ProcessAnaTuples.  I recommend you store any cut ranges as member variables in the constructor based on YAML information and then use them in `checkCut()`/`passesCut()`.  Finally, you have to do two things to make a new Cut visible to ProcessAnaTuples:
- Register it at the bottom of its .cpp file with lines like these:
```
namespace
{
  static reco::Cut::Registrar<reco::HasInteractionVertex> HasInteractionVertex_reg("HasInteractionVertex");
}
```
  Replace `reco::HasInteractionVertex` with the class name of your Cut.  Replace `reco::` with `truth::` if writing a truth cut/signal constraint.
- Add it to the end of the `add_library()` line in `cuts/reco/CMakeLists.txt` or `cuts/truth/CMakeLists.txt`.

### How to Write a Reweighter
MINERvA simulates different physics models by e.g. counting the same simulated event multiple times.  This saves the vast majority of computing time that goes into analyzing a new data set.  Reweighter is how ProcessAnaTuples adds a physics effect to a Model.  To add a new reweighting option, the main function you need to override is `double GetWeight(const UNIVERSE& univ, const EVENT& /*event*/) const override`.  Much like adding a Cut, you can control a Reweighter from a YAML file by writing a constructor for it.  PlotUtils::Reweighter also has two functions to give ProcessAnaTuples more information about your class at the beginning and end of the event loop:
- `std::string GetName() const override`: Usually a one-line function that returns a string that identifies your Reweighter
- `bool DependsReco() const override`: Must return `true` if your Reweighter uses any reconstructed quantities.  If you return false but use reconstructed quantities anyway, you will get the wrong physics!  With that said, the vast majority of use cases don't need to return true here.

Finally, you need to do two things to make your Reweighter available in ProcessAnaTuples:
- Register it at the bottom of its .cpp file like this:
```namespace
{
  plgn::Registrar<PlotUtils::Reweighter<evt::Universe, PlotUtils::detail::empty>, NuWroSFReweighter<evt::Universe, PlotUtils::detail::empty>> reg_NuWroSFReweight("NuWroSFReweighter");
}
```
- Add it to the end of the `add_library()` list in `reweighters/CMakeLists.txt`

If you want to use a **new Reweighter from PlotUtils** with ProcessAnaTuples, you need to add a "registrar" class for it to the bottom of `reweighters/RegisterReweighters.h`.  This is basically a layer that translates a YAML file into c++ numbers and file names.  See that files for examples.
