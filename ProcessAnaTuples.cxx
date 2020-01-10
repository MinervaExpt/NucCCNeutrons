//File: ProcessAnaTuples.cxx
//Brief: A program to make plots from NucCCNeutrons NTuples.  Infers
//       whether first file is data or MC based on whether Truth tree
//       is available and then asserts that this is true for all other
//       files.  The target and analysis cuts it uses need to be configured
//       by a YAML file.
//
//       Based on NewIO with experimental support for systematics and modularized Cuts.
//
//       Usage:
//       ProcessAnaTuples <yourCuts.yaml> [moreConfigsInOrder.yaml]... <yourTuple.root> [moreTuples.root]..."
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/CVUniverse.h"

//utility includes
#include "util/Factory.cpp"

//analysis includes
#include "signal/Signal.h"
#include "sideband/Sideband.h"
#include "background/Background.h"

//Cuts includes
#include "cuts/truth/Cut.h"
#include "cuts/reco/Cut.h"

//app includes
#include "app/CmdLine.h"
#include "app/IsMC.h"

//PlotUtils includes
#include "PlotUtils/CrashOnROOTErrorMessage.h"

//YAML-cpp includes
#include "yaml-cpp/yaml.h"

//ROOT includes
#include "TFile.h"

//c++ includes
#include <iostream>

//Helper functions to make main() simpler
namespace
{
  //How often I print out entry number when in debug mode
  constexpr auto printFreq = 1000ul;

  template <class CUT>
  std::vector<std::unique_ptr<CUT>> loadPlugins(const YAML::Node& config)
  {
    std::vector<std::unique_ptr<CUT>> cuts;
    auto& factory = plgn::Factory<CUT>::instance();

    for(auto block: config) cuts.emplace_back(block.first, factory.Get(block.second)); //TODO: block.first needs to be passed to factory or else I need to associate names to CUTS some other way.

    return cuts;
  }

  int hashCuts(const std::vector<std::string> cutNames, std::vector<reco::Cut>& allCuts, std::vector<reco::Cut>& sidebandCuts)
  {
    uint64_t result = 0x1 << cuts.size() - 1;

    for(const auto& name: cutNames)
    {
      const auto found = std::find_if(allCuts.begin(), allCuts.end(), [&name](const auto& cut) { return cut.name() == name; });
      if(found != allCuts.end())
      {
        result |= 0x1 << sidebandCuts.size(); //BEFORE the update because of 0-based indexing
        //TODO: Move found to the end of sidebandCuts
      }
      else
      {
        const auto inSideband = std::find_if(sidebandCuts.begin(), sidebandCuts.end(), [&name](const auto& cut) { return cut.name() == name; });
        if(inSideband != sidebandCuts.end())
        {
          result |= 0x1 << std::distance(sidebandCuts.begin(), inSideband);
        } //If in sidebandCuts
      } //If not in allCuts
    } //For each cut name

    return result;
  }

  //TODO: Since I've got to do this to dereference std::unique_ptr<CUT>, I could give Cut's cut function a more descriptive name
  template <class CUT>
  bool requireAll(const std::vector<std::unique_ptr<CUT>>& cuts, const evt::CVUniverse& event)
  {
    return std::all_of(cuts.begin(), cuts.end(),
                       [&event](const auto cut)
                       {
                         return (*cut)(event);
                       });
  }
}

int main(const int argc, const char** argv)
{
  //TODO: Move these parameters somehwere that can be shared between applications?
  constexpr auto anaTupleName = "NucCCNeutron";

  try
  {
    const apo::CmdLine options(argc, argv); //Parses the command line for input and configuration file, assembles a
                                            //list of files to process, prepares a file for histograms, and puts the configuration
                                            //file together.  See CmdLine.h for more details.

    //Set up other application options
    const auto& appOptions = options.ConfigFile()["app"];
    const auto blobAlg = appOptions["BlobAlg"].as<std::string>("proximity");

    //The file where I will put histrograms I produce.
    //TODO: I learned when helping Christian that I can't use TFile::Write() to write
    //      MnvH1Ds' plots because MnvH1D insists on deleteing its own TH1Ds.  I've got
    //      to either Write() each TH1D or convince TFile not to delete the objects it
    //      owns.  I think there's a flag for the latter in TObject.
    const bool isThisJobMC = app::IsMC(options.TupleFileNames().front());
    TFile histFile((std::string(argv[0]) + "_" + (isThisJobMC?"MC":"Data")).c_str(), "CREATE");
    util::Directory histDir(histFile);

    //Set up Signal
    std::unique_ptr<Signal> signal(histDir.mkdir(options["signal"]["name"].as<std::string>()), options["signal"]);

    //Set up cuts, sidebands, and backgrounds
    auto truthCuts = ::loadPlugins<TruthCut>(options["cuts"]["truth"]);
    auto recoCuts = ::loadPlugins<RecoCut>(options["cuts"]["reco"]);
    decltype(recoCuts) sidebandCuts;

    //TODO: Put these extended setup steps into functions?
    std::unordered_map<int, std::vector<std::unique_ptr<Sideband>>> cutsToSidebands; //Mapping from truth cuts to sidebands
    for(auto sideband: config["sidebands"])
    {
      const auto& fails = sideband.second["fails"].as<std::vector<std::string>>();
      const auto passes = ::loadPlugins<reco::Cut>(sideband.second["passes"]);
      const auto hashPattern = ::hashCuts(fails, recoCuts, sidebandCuts); //Also transfers relevant cuts from recoCuts to sidebandCuts
      cutsToSidebands[hashPattern].emplace_back(new Sideband(histDir.mkdir(sideband.first.as<std::string>()), std::move(passes)));
    }

    std::vector<std::unique_ptr<Background>> backgrounds;
    for(auto background: config["backgrounds"])
    {
      const auto& passes = ::loadPlugins<truth::Cut>(background.second);
      backgrounds.emplace_back(new Background(histDir.mkdir(background.first.as<std::string>()), std::move(passes)));
    }

    //Accumulate POT from each good file
    double pot_used = 0;

    //TODO: Loop over files
    for(const auto& fName: options.TuplefileNames())
    {
      //Sanity checks on AnaTuple files
      std::unique_ptr<TFile> tupleFile = TFile::Open(fName.c_str());
      if(tupleFile == nullptr)
      {
        std::cerr << fName << ": No such file or directory.  Skipping this "
                  << "file name.\n";
        continue; //TODO: Don't use break if I can help it
      }

      //TODO: Doesn't my ROOT error checking function throw an exception here?
      if(app::IsMC(fName) != isThisJobMC)
      {
        std::cerr << "This job " << (isThisJobMC?"is":"is not")
                  << " processing MC files, but " << fName << " is a "
                  << (app::IsMC(fName)?"MC":"data")
                  << "file.  Skipping this file!\n";
        continue; //TODO: Don't use break if I can help it
      }

      auto meta = static_cast<TTree&>(*tupleFile->Get("Meta"));
      if(meta == nullptr)
      {
        std::cerr << fName << " does not contain POT information!  This might be a merging failure.  Skipping this file.\n";
        continue; //TODO: Don't use break if I can help it
      }

      //Get POT for this file, but don't accumulate it until I've found the
      //other trees I need.
      PlotUtils::TreeWrapper meta(tupleFile, "Meta");
      const double thisFilesPOT = meta.GetDouble("POT_Used");

      //TODO: Make this a TreeWrapper to avoid opening fName twice
      PlotUtils::ChainWrapper anaTuple(anaTupleName);

      try
      {
        anaTuple.AddFile(fName);
      }
      catch(const PlotUtils::ChainWrapper::BadFile& err)
      {
        std::cerr << "Failed to find an AnaTuple named " << anaTupleName
                  << " in " << fName << ".  Skipping this file name.\n";
        continue; //TODO: Don't use continue if I can help it
      }

      //On to the event loops
      if(isThisJobMC)
      {
        //Check for Truth tree
        //TODO: ChainWrapper -> TreeWrapper to avoid opening tupleFile again
        PlotUtils::ChainWrapper truthTree("Truth");

        try
        {
          truthTree.AddFile(fName);
        }
        catch(const PlotUtils::ChainWrapper::BadFile& err)
        {
          std::cerr << "Failed to find an AnaTuple named Truth "
                    << " in " << fName << ".  Skipping this file name.\n";
          continue; //TODO: Don't use continue if I can help it
        }

        //MC loop
        const size_t nMCEntries = anaTuple.GetEntries();
        std::vector<evt::CVUniverse> mcUniverses = {evt::CVUniverse(blobAlg, &anaTuple)}; //TODO: Set up full list of systematic universes

        for(size_t entry = 0; entry < nEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with MC entry " << entry << "\n";
          #endif

          for(const auto& event: mcUniverses)
          {
            event->SetEntry(entry);

            //MC loop
            if(::requireAll(recoCuts, event) && ::requireAll(truthCuts, event))
            {
              //Bitfields encoding which truth and reco cuts I passed.  Effectively, this hashes sidebands in a way that works even
              //for sidebands defined by multiple cuts.
              uint64_t passedReco = 0; //TODO: What does the default value for each bit need to be to work with XOR when I have fewer than 64 cuts?
              constexpr decltype(passedTruth) passedAll = 1 << sidebandCuts.size() - 1; //TODO: Change this to somehting like 0b1 << nCuts - 1

              for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
              {
                const auto& cut = sidebandCuts[whichCut];
                passedReco |= (*cut)(event) << whichCut;
              }

              if(!(passedReco ^ passedAll)) signal->mc(event);
              else
              {
                const auto foundSidebands = sidebands.find(passedReco); //Look up the sideband, if any, by which reco cuts failed
                const auto foundBackground = std::find_if(backgrounds.begin(), backgrounds.end(),
                                                          [&event](const auto& background)
                                                          { return ::requireAll(background->passes, event); });

                if(foundSidebands != sidebands.end())
                {
                  const auto firstSideband = std::find_if(foundSidebands->begin(), foundSidebands->end(),
                                                          [&event](const auto sideband)
                                                          { return ::requireAll(sideband->passes, event); });

                  if(firstSideband != foundSidebands->end())
                  {
                    if(!(passedTruth ^ passedAll)) firstSideband->FillSignal(event); //TODO: What's the deal with this line?  Do I just not require any truth cuts at all for sideband background?
                    else (*foundSideband)->FillBackground(event, foundBackground); //TODO: Make sure backgrounds.end() is the "Other" category
                  }
                  //TODO: foundSideband on the previous line needs to know how to look up a background from foundBackground somehow.
                  //      I guess I can just create the list of backgrounds first?
                  //      Also, it seems like I've just re-invented my ideas about categorization.
                }
                if(foundBackground != backgrounds.end()) foundBackground->Fill(event);
              } //If MC is not reco and truth signal
            } //If passed all non-sideband-related cuts
          } //For each MC systematic universe
        } //For each entry in the MC tree

        //Truth loop
        const size_t nTruthEntires = truthTree.GetEntries();
        std::vector<evt::CVUniverse> truthUniverses = {evt::CVUniverse(blobAlg, &truthTree)}; //TODO: Set up fill list of systematic universes

        for(size_t entry = 0; entry < nEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with truth entry " << entry << "\n";
          #endif

          for(const auto& event: truthUniverses)
          {
            event->SetEntry(entry);

            if(::requireAll(truthCuts, event))
            {
              signal->truth(event);
            } //If event passes all truth cuts
          } //For each systematic universe
        } //For each entry in Truth tree
      } //If isThisJobMC
      else
      {
        //Data loop
        const size_t nEntries = anaTuple->GetEntries();
        evt::CVUniverse event(blobAlg, anaTuple);

        for(size_t entry = 0; entry < nEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with data entry " << entry << "\n";
          #endif

          event->setEntry(entry);
          if(::requireAll(recoCuts, event))
          {
            uint64_t passedCuts = 0;
            constexpr decltype(passedCuts) passedAll = 1 << sidebandCuts.size() - 1; //TODO: Is 0 an appropriate default with XOR?

            for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
            {
              passedCuts |= sidebandCuts[whichCut](event) << whichCut;
            }

            if(!(passedCuts ^ passedAll)) signal->FillData(event);
            else
            {
              const auto foundSidebands = sidebands.find(passedCuts);
              if(foundSidebands != sidebands.end()
              {
                const auto firstSideband = std::find_if(foundSidebands->begin(), foundSidebands->end(),
                                                        [&event](const auto sideband)
                                                        { return ::requireAll(sideband->passes, event); });
                if(firstSideband != foundSidebands->end()) firstSideband->FillData(event);
              } //If found a sideband and passed all of its reco constraints
            } //If not passed all sideband-related cuts
          } //If passed all cuts not related to sidebands
        } //For each entry in data tree
      } //If not isThisJobMC

      //I've finished with this file, so I guess I read it sucessfully.  Time to count its POT.
      pot_used += thisFilesPOT;
    } //For each AnaTuple file
  } //try-catch on whole event loop
  catch(const apo::CmdLine::exception& e)
  {
    std::cerr << e.what() << "\nReturning " << e.reason << " without doing anything!\n";
    return e.reason;
  }
  //If I ever want to handle ROOT warnings differently, this is
  //the place to implement that behavior.
  catch(const ROOT::Warning& e)
  {
    std::cerr << e.what() << "\nExiting immediately, so you probably got incomplete results!";
    return apo::CmdLine::ExitCode::IOError;
  }
  catch(const ROOT::Error& e)
  {
    std::cerr << e.what() << "\nExiting immediately, so you probably got incomplete results!";
    return apo::CmdLine::ExitCode::IOError;
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Got a fatal std::runtime_error while running the analysis:\n"
              << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return apo::CmdLine::ExitCode::AnalysisError;
  }

  return apo::CmdLine::ExitCode::Success;
}
