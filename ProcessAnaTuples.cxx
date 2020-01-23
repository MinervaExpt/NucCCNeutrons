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

#define PLOTUTILS_THROW_EXCEPTIONS

//evt includes
#include "evt/CVUniverse.h"

//utility includes
#include "util/Factory.cpp"

//analysis includes
#include "analyses/signal/Signal.h"
#include "analyses/sideband/Sideband.h"
#include "analyses/Background.h"

//Cuts includes
#include "cuts/truth/Cut.h"
#include "cuts/reco/Cut.h"

//app includes
#include "app/CmdLine.h"
#include "app/IsMC.h"

//PlotUtils includes
#include "PlotUtils/CrashOnROOTMessage.h"

//YAML-cpp includes
#include "yaml-cpp/yaml.h"

//ROOT includes
#include "TFile.h"
#include "TTree.h"

//c++ includes
#include <iostream>
#include <algorithm>
#include <unordered_map>

//Helper functions to make main() simpler
namespace
{
  //How often I print out entry number when in debug mode
  constexpr auto printFreq = 1000ul;

  std::unique_ptr<bkg::Background> null(nullptr); //TODO: This is a horrible hack so I can hold a reference to a nullptr

  template <class ITERATOR>
  auto derefOrNull(const ITERATOR it, const ITERATOR end) -> decltype(*it)
  {
    if(it == end) return ::null;
    return *it;
  }

  int hashCuts(const std::vector<std::string>& cutNames, std::vector<std::unique_ptr<reco::Cut>>& allCuts, std::vector<std::unique_ptr<reco::Cut>>& sidebandCuts)
  {
    uint64_t result = 0; //0x1 << sidebandCuts.size() - 1;

    for(const auto& name: cutNames)
    {
      const auto found = std::find_if(allCuts.begin(), allCuts.end(), [&name](const auto& cut) { return cut->name() == name; });
      if(found != allCuts.end())
      {
        result |= 0x1 << sidebandCuts.size(); //BEFORE the update because of 0-based indexing
        //TODO: Move found to the end of sidebandCuts
      }
      else
      {
        const auto inSideband = std::find_if(sidebandCuts.begin(), sidebandCuts.end(), [&name](const auto& cut) { return cut->name() == name; });
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
                       [&event](const auto& cut)
                       {
                         return (*cut)(event);
                       });
  }

  std::vector<evt::CVUniverse*> getSystematics(PlotUtils::ChainWrapper* chw, const bool /*isMC*/)
  {
    //TODO: Get centralized systematics if !isMC
    return {new evt::CVUniverse(chw)};
  }
}

int main(const int argc, const char** argv)
{
  //TODO: Move these parameters somehwere that can be shared between applications?
  constexpr auto anaTupleName = "NucCCNeutron";

  try
  {
    const app::CmdLine options(argc, argv); //Parses the command line for input and configuration file, assembles a
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

    //Decide which systematic universes to process, but delay setting the tree to
    //process until later.
    //TODO: Can't we just agree to use std::unique_ptr<>s instead?  Now, I've got
    //      to remember to delete these.
    std::vector<evt::CVUniverse*> universes = ::getSystematics(nullptr, isThisJobMC);

    //Set up Signal
    auto signalDir =  histDir.mkdir(options.ConfigFile()["signal"]["name"].as<std::string>());
    auto signal = plgn::Factory<sig::Signal, util::Directory&, std::vector<evt::CVUniverse*>&>::instance().Get(options.ConfigFile()["signal"], signalDir, universes);

    //Set up cuts, sidebands, and backgrounds
    auto truthPhaseSpace = plgn::loadPlugins<truth::Cut>(options.ConfigFile()["cuts"]["truth"]["phaseSpace"]);
    auto truthSignal = plgn::loadPlugins<truth::Cut>(options.ConfigFile()["cuts"]["truth"]["signal"]);
    std::vector<std::unique_ptr<reco::Cut>> recoCuts;
    
    auto& cutFactory = plgn::Factory<reco::Cut, std::string&>::instance();
    for(auto config: options.ConfigFile()["cuts"]["reco"])
    {
      auto name = config.first.as<std::string>();
      recoCuts.emplace_back(cutFactory.Get(config.second, name));
    }

    decltype(recoCuts) sidebandCuts;

    //TODO: Put these extended setup steps into functions?
    std::vector<std::unique_ptr<bkg::Background>> backgrounds;
    for(const auto config: options.ConfigFile()["backgrounds"]) backgrounds.emplace_back(new bkg::Background(config.first.as<std::string>(), config.second));

    std::unordered_map<int, std::vector<std::unique_ptr<side::Sideband>>> sidebands; //Mapping from truth cuts to sidebands
    auto& sideFactory = plgn::Factory<side::Sideband, util::Directory&, std::vector<std::unique_ptr<reco::Cut>>&&, std::vector<std::unique_ptr<bkg::Background>>&, std::vector<evt::CVUniverse*>&>::instance();
    for(auto sideband: options.ConfigFile()["sidebands"])
    {
      auto dir = histDir.mkdir(sideband.first.as<std::string>());
      const auto& fails = sideband.second["fails"].as<std::vector<std::string>>();
      auto passes = plgn::loadPlugins<reco::Cut>(sideband.second["passes"]);
      const auto hashPattern = ::hashCuts(fails, recoCuts, sidebandCuts); //Also transfers relevant cuts from recoCuts to sidebandCuts
      sidebands[hashPattern].emplace_back(sideFactory.Get(sideband.second, dir, std::move(passes), backgrounds, universes));
    }

    //Accumulate POT from each good file
    double pot_used = 0;

    //TODO: Loop over files
    for(const auto& fName: options.TupleFileNames())
    {
      //Sanity checks on AnaTuple files
      std::unique_ptr<TFile> tupleFile(TFile::Open(fName.c_str())); //TODO: Make sure this TFile is written?  Seems like I have to Write() all MnvH1Ds.
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

      auto metaTree = dynamic_cast<TTree*>(tupleFile->Get("Meta"));
      if(!metaTree)
      {
        std::cerr << fName << " does not contain POT information!  This might be a merging failure.  Skipping this file.\n";
        continue; //TODO: Don't use break if I can help it
      }

      //Get POT for this file, but don't accumulate it until I've found the
      //other trees I need.
      PlotUtils::TreeWrapper meta(metaTree);
      const double thisFilesPOT = meta.GetValue("POT_Used", 0);

      //TODO: Make this a TreeWrapper to avoid opening fName twice
      PlotUtils::ChainWrapper anaTuple(anaTupleName);

      try
      {
        anaTuple.Add(fName);
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
          truthTree.Add(fName);
        }
        catch(const PlotUtils::ChainWrapper::BadFile& err)
        {
          std::cerr << "Failed to find an AnaTuple named Truth "
                    << " in " << fName << ".  Skipping this file name.\n";
          continue; //TODO: Don't use continue if I can help it
        }

        //MC loop
        const size_t nMCEntries = anaTuple.GetEntries();
        for(auto& univ: universes) univ->SetTree(&anaTuple);

        for(size_t entry = 0; entry < nMCEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with MC entry " << entry << "\n";
          #endif

          for(const auto univ: universes)
          {
            auto& event = *univ;
            event.SetEntry(entry);

            //MC loop
            if(::requireAll(recoCuts, event))
            {
              //Bitfields encoding which reco cuts I passed.  Effectively, this hashes sidebands in a way that works even
              //for sidebands defined by multiple cuts.
              uint64_t passedReco = 0; //TODO: What does the default value for each bit need to be to work with XOR when I have fewer than 64 cuts?
              const decltype(passedReco) passedAll = (1 << sidebandCuts.size()) - 1;

              for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
              {
                const auto& cut = sidebandCuts[whichCut];
                passedReco |= (*cut)(event) << whichCut;
              }

              //Look up the sideband, if any, by which reco cuts failed.
              //I might not find any sideband if this event is reconstructed signal
              //or just isn't in a sideband I'm interested in.
              //Putting the code here just makes it easier to maintain.
              const auto relevantSidebands = sidebands.find(passedReco);
              side::Sideband* sideband = nullptr; //TODO: Come up with some clever return value semantics to avoid the raw pointer here
              if(relevantSidebands != sidebands.end())
              {
                const auto found = std::find_if(relevantSidebands->second.begin(), relevantSidebands->second.end(),
                                                [&event](const auto& whichSideband)
                                                { return ::requireAll(whichSideband->passes, event); });
                if(found != relevantSidebands->second.end()) sideband = found->get();
              }

              //Categorize by whether this is signal or some background
              if(::requireAll(truthSignal, event))
              {
                if(!(passedReco ^ passedAll)) signal->mcSignal(event);
                else if(sideband) //If this is a sideband I'm interested in
                {
                  sideband->truthSignal(event);
                }
              }
              else //If not truthSignal
              {
                const auto foundBackground = std::find_if(backgrounds.begin(), backgrounds.end(),
                                                          [&event](const auto& background)
                                                          { return ::requireAll(background->passes, event); });

                if(!(passedReco ^ passedAll)) signal->mcBackground(event, ::derefOrNull(foundBackground, backgrounds.end()));
                else sideband->truthBackground(event, ::derefOrNull(foundBackground, backgrounds.end()));
              }
            } //If passed all non-sideband-related cuts
          } //For each MC systematic universe
        } //For each entry in the MC tree

        //Truth loop
        const size_t nTruthEntries = truthTree.GetEntries();
        for(auto& univ: universes) univ->SetTree(&truthTree);

        for(size_t entry = 0; entry < nTruthEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with truth entry " << entry << "\n";
          #endif

          for(const auto univ: universes)
          {
            auto& event = *univ;
            event.SetEntry(entry);

            if(::requireAll(truthPhaseSpace, event) && ::requireAll(truthSignal, event))
            {
              signal->truth(event);
            } //If event passes all truth cuts
          } //For each systematic universe
        } //For each entry in Truth tree
      } //If isThisJobMC
      else
      {
        //Data loop
        const size_t nEntries = anaTuple.GetEntries();
        auto& cv = universes.front(); //TODO: assert() that this is true?  Put data in a different program?
        cv->SetTree(&anaTuple);

        for(size_t entry = 0; entry < nEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with data entry " << entry << "\n";
          #endif

          auto& event = *cv;
          event.SetEntry(entry);
          if(::requireAll(recoCuts, event))
          {
            uint64_t passedCuts = 0;
            const decltype(passedCuts) passedAll = (1 << sidebandCuts.size()) - 1; //TODO: Is 0 an appropriate default with XOR?

            for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
            {
              passedCuts |= (*sidebandCuts[whichCut])(event) << whichCut;
            }

            if(!(passedCuts ^ passedAll)) signal->data(event);
            else
            {
              const auto foundSidebands = sidebands.find(passedCuts);
              if(foundSidebands != sidebands.end())
              {
                const auto firstSideband = std::find_if(foundSidebands->second.begin(), foundSidebands->second.end(),
                                                        [&event](const auto& sideband)
                                                        { return ::requireAll(sideband->passes, event); });
                if(firstSideband != foundSidebands->second.end()) (*firstSideband)->data(event);
              } //If found a sideband and passed all of its reco constraints
            } //If not passed all sideband-related cuts
          } //If passed all cuts not related to sidebands
        } //For each entry in data tree
      } //If not isThisJobMC

      //I've finished with this file, so I guess I read it sucessfully.  Time to count its POT.
      pot_used += thisFilesPOT;
    } //For each AnaTuple file
  } //try-catch on whole event loop
  catch(const app::CmdLine::exception& e)
  {
    std::cerr << e.what() << "\nReturning " << e.reason << " without doing anything!\n";
    return e.reason;
  }
  //If I ever want to handle ROOT warnings differently, this is
  //the place to implement that behavior.
  catch(const ROOT::warning& e)
  {
    std::cerr << e.what() << "\nExiting immediately, so you probably got incomplete results!";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const ROOT::error& e)
  {
    std::cerr << e.what() << "\nExiting immediately, so you probably got incomplete results!";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Got a fatal std::runtime_error while running the analysis:\n"
              << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::AnalysisError;
  }

  return app::CmdLine::ExitCode::Success;
}
