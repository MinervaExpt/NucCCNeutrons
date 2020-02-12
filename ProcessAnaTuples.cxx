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
#include "PlotUtils/GenieSystematics.h"
#include "PlotUtils/FluxSystematics.h"

//YAML-cpp includes
#include "yaml-cpp/yaml.h"

//ROOT includes
#include "TFile.h"
#include "TTree.h"

//Cintex is only needed for older ROOT versions like the GPVMs.
//Let CMake decide whether it's needed.
#ifndef NCINTEX
#include "Cintex/Cintex.h"
#endif

//c++ includes
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <bitset>

//Macro to centralize how I print out debugging messages
//TODO: Decide how I want this macro to work and centralize it.
#ifndef NDEBUG
  #define LOG_DEBUG(msg) std::cerr << msg << "\n";
#else
  #define LOG_DEBUG(msg)
#endif

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

  //Given the names of cuts for a sideband, all reco cuts, and the cuts already associated with sidebands,
  //create a hash code that describes which cuts an event must fail to be
  //in this sideband and move any needed cuts from allCuts to sidebandCuts.
  std::bitset<64> hashCuts(const std::vector<std::string>& cutNames, std::vector<std::unique_ptr<reco::Cut>>& allCuts, std::vector<std::unique_ptr<reco::Cut>>& sidebandCuts)
  {
    std::bitset<64> result;
    result.set(); //Sets result to all true

    for(const auto& name: cutNames)
    {
      const auto found = std::find_if(allCuts.begin(), allCuts.end(), [&name](const auto& cut) { return cut->name() == name; });
      if(found != allCuts.end())
      {
        result.set(sidebandCuts.size(), false); //BEFORE the update because of 0-based indexing
        sidebandCuts.push_back(std::move(*found));
        allCuts.erase(found);
      }
      else
      {
        const auto inSideband = std::find_if(sidebandCuts.begin(), sidebandCuts.end(), [&name](const auto& cut) { return cut->name() == name; });
        if(inSideband != sidebandCuts.end())
        {
          result.set(std::distance(sidebandCuts.begin(), inSideband), false);
        } //If in sidebandCuts
        else throw std::runtime_error("Failed to find a cut named " + name + " for a sideband.\n");
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

  //TODO: Replace with a "universe loader" that loads systematic universes from a file.
  std::map<std::string, std::vector<evt::CVUniverse*>> getSystematics(PlotUtils::ChainWrapper* chw, const app::CmdLine& options, const bool isMC)
  {
    std::map<std::string, std::vector<evt::CVUniverse*>> result;
    result["cv"].push_back(new evt::CVUniverse(chw));

    //"global" configuration for all DefaultCVUniverses
    DefaultCVUniverse::SetPlaylist(options.playlist());
    DefaultCVUniverse::SetAnalysisNuPDG(-14); //TODO: Get this from the user somehow
    DefaultCVUniverse::SetNuEConstraint(false); //No nu-e constraint for antineutrino mode yet
    DefaultCVUniverse::SetNonResPiReweight(false);

    if(isMC)
    {
      //TODO: Turn this universes back on when I'm ready to deal with weights in a reasonable way
      /*const int nFluxUniverses = 50; //TODO: Get this number from the user and tune it
      DefaultCVUniverse::SetNFluxUniverses(nFluxUniverses);
      auto fluxSys = PlotUtils::GetFluxSystematicsMap<evt::CVUniverse>(chw, nFluxUniverses);
      result.insert(fluxSys.begin(), fluxSys.end());

      auto genieSyst = PlotUtils::GetGenieSystematicsMap<evt::CVUniverse>(chw);
      result.insert(genieSyst.begin(), genieSyst.end());*/
    }

    return result;
  }

  //Some systematic universes return true from IsVerticalOnly().  Those universes are guaranteed by the NS Framework to
  //return the same physics variables as the CV EXCEPT FOR GETWEIGHT().  So, group them together to save on
  //evaluating cuts and physics variables.
  std::vector<std::vector<evt::CVUniverse*>> groupCompatibleUniverses(const std::map<std::string, std::vector<evt::CVUniverse*>> bands)
  {
    std::vector<std::vector<evt::CVUniverse*>> groupedUnivs(1);

    auto& vertical = groupedUnivs[0]; //By convention, put vertical universes at the beginning of this vector<>.
                                      //This is an implementation detail that may change at any time.

    for(const auto& band: bands)
    {
      if(band.first == "cv") vertical.insert(vertical.end(), band.second.begin(), band.second.end());
      else
      {
        for(const auto univ: band.second)
        {
          if(univ->IsVerticalOnly()) vertical.push_back(univ);
          else groupedUnivs.emplace_back(std::vector<evt::CVUniverse*>{univ});
        }
      }
    }

    return groupedUnivs;
  }
}

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(kFALSE); //Needed so that MnvH1D gets to clean up its own MnvLatErrorBands (which are TH1Ds).

  //TODO: Move these parameters somehwere that can be shared between applications?
  constexpr auto anaTupleName = "NucCCNeutron";

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
  const bool isThisJobMC = options.isMC();
  util::Directory histDir(*options.HistFile);

  //Decide which systematic universes to process, but delay setting the tree to
  //process until later.
  //TODO: Can't we just agree to use std::unique_ptr<>s instead?  Now, I've got
  //      to remember to delete these.
  auto universes = ::getSystematics(nullptr, options, isThisJobMC);

  //Set up backgrounds
  LOG_DEBUG("Setting up Backgrounds...")
  std::vector<std::unique_ptr<bkg::Background>> backgrounds;
  try
  {
    for(const auto config: options.ConfigFile()["backgrounds"]) backgrounds.emplace_back(new bkg::Background(config.first.as<std::string>(), config.second));
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to setup up a Background:\n" << e.what() << "\n";
    return app::CmdLine::ExitCode::YAMLError;
  }

  //Set up Signal
  LOG_DEBUG("Setting up Signal...")
  auto signalDir = histDir.mkdir(options.ConfigFile()["signal"]["name"].as<std::string>());
  std::unique_ptr<sig::Signal> signal;
  try
  {
    signal = plgn::Factory<sig::Signal, util::Directory&, std::vector<std::unique_ptr<bkg::Background>>&, std::map<std::string, std::vector<evt::CVUniverse*>>&>::instance().Get(options.ConfigFile()["signal"], signalDir, backgrounds, universes);
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to set up the Signal:\n" << e.what() << "\n";
    return app::CmdLine::ExitCode::YAMLError;
  }

  //Set up cuts, sidebands, and backgrounds
  LOG_DEBUG("Setting up truth cuts...")
  std::vector<std::unique_ptr<truth::Cut>> truthPhaseSpace;
  try
  {
    truthPhaseSpace = plgn::loadPlugins<truth::Cut>(options.ConfigFile()["cuts"]["truth"]["phaseSpace"]);
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to set up a phase space truth::Cut:\n" << e.what() << "\n";
    return app::CmdLine::ExitCode::YAMLError;
  }

  std::vector<std::unique_ptr<truth::Cut>> truthSignal;
  try
  {
    truthSignal = plgn::loadPlugins<truth::Cut>(options.ConfigFile()["cuts"]["truth"]["signal"]);
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to set up a signal definition truth::Cut:\n" << e.what() << "\n";
    return app::CmdLine::ExitCode::YAMLError;
  }
  
  LOG_DEBUG("Setting up reco cuts...")
  std::vector<std::unique_ptr<reco::Cut>> recoCuts;
  auto& cutFactory = plgn::Factory<reco::Cut, std::string&>::instance();
  try
  {
    for(auto config: options.ConfigFile()["cuts"]["reco"])
    {
      auto name = config.first.as<std::string>();
      recoCuts.emplace_back(cutFactory.Get(config.second, name));
    }
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Failed to set up a reco::Cut:\n" << e.what() << "\n";
    return app::CmdLine::ExitCode::YAMLError;
  }

  LOG_DEBUG("Setting up sidebands...")
  decltype(recoCuts) sidebandCuts;

  std::unordered_map<std::bitset<64>, std::vector<std::unique_ptr<side::Sideband>>> sidebands; //Mapping from truth cuts to sidebands
  auto& sideFactory = plgn::Factory<side::Sideband, util::Directory&, std::vector<std::unique_ptr<reco::Cut>>&&, std::vector<std::unique_ptr<bkg::Background>>&, std::map<std::string, std::vector<evt::CVUniverse*>>&>::instance();
  for(auto sideband: options.ConfigFile()["sidebands"])
  {
    try
    {
      auto dir = histDir.mkdir(sideband.first.as<std::string>());
      const auto& fails = sideband.second["fails"].as<std::vector<std::string>>();

      std::vector<std::unique_ptr<reco::Cut>> passes;
      for(const auto config: sideband.second["passes"])
      {
        auto name = config.first.as<std::string>();
        try
        {
          passes.emplace_back(cutFactory.Get(config.second, name));
        }
        catch(const std::runtime_error& e)
        {
          std::cerr << "Failed to set up a reco::Cut named " << name << " for Sideband " << sideband.first.as<std::string>() << ":\n" << e.what() << "\n";
          return app::CmdLine::ExitCode::YAMLError;
        }
      }

      const auto hashPattern = ::hashCuts(fails, recoCuts, sidebandCuts); //Also transfers relevant cuts from recoCuts to sidebandCuts
      sidebands[hashPattern].emplace_back(sideFactory.Get(sideband.second, dir, std::move(passes), backgrounds, universes));
    }
    catch(const std::runtime_error& e)
    {
      std::cerr << "Failed to set up a Sideband named " << sideband.first.as<std::string>() << ":\n" << e.what() << "\n";
      return app::CmdLine::ExitCode::YAMLError;
    }
  }

  //TODO: Can I let universes go out of scope now to make the event loop simpler?
  const auto groupedUnivs = ::groupCompatibleUniverses(universes);

  //Accumulate POT from each good file
  double pot_used = 0;

  //Loop over files
  LOG_DEBUG("Beginning loop over files!")
  try
  {
    for(const auto& fName: options.TupleFileNames())
    {
      LOG_DEBUG("Loading " << fName)
      //Sanity checks on AnaTuple files
      std::unique_ptr<TFile> tupleFile(TFile::Open(fName.c_str()));
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
        for(auto& compat: groupedUnivs)
        {
          for(auto& univ: compat) univ->SetTree(&anaTuple);
        }

        //Get MINOS weights
        PlotUtils::DefaultCVUniverse::SetTruth(false);

        for(size_t entry = 0; entry < nMCEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with MC entry " << entry << "\n";
          #endif

          for(const auto& compat: groupedUnivs)
          {
            auto& event = *compat.front(); //All compatible universes pass the same cuts
            for(const auto univ: compat) univ->SetEntry(entry); //I still need to GetWeight() for entry

            //MC loop
            if(::requireAll(recoCuts, event))
            {
              //Bitfields encoding which reco cuts I passed.  Effectively, this hashes sidebands in a way that works even
              //for sidebands defined by multiple cuts.
              std::bitset<64> passedReco;
              passedReco.set();

              for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
              {
                const auto& cut = sidebandCuts[whichCut];
                if(!(*cut)(event)) passedReco.set(whichCut, false);
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
                if(passedReco.all()) signal->mcSignal(compat);
                else if(sideband) //If this is a sideband I'm interested in
                {
                  sideband->truthSignal(compat);
                }
              }
              else //If not truthSignal
              {
                const auto foundBackground = std::find_if(backgrounds.begin(), backgrounds.end(),
                                                          [&event](const auto& background)
                                                          { return ::requireAll(background->passes, event); });

                if(passedReco.all()) signal->mcBackground(compat, ::derefOrNull(foundBackground, backgrounds.end()));
                else if(sideband) sideband->truthBackground(compat, ::derefOrNull(foundBackground, backgrounds.end()));
              }
            } //If passed all non-sideband-related cuts
          } //For each error band
        } //For each entry in the MC tree

        //Truth loop
        const size_t nTruthEntries = truthTree.GetEntries();
        for(auto& compat: groupedUnivs)
        {
          for(auto univ: compat) univ->SetTree(&truthTree);
        }

        //Don't try to get MINOS weights in the truth tree loop
        PlotUtils::DefaultCVUniverse::SetTruth(true);

        for(size_t entry = 0; entry < nTruthEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with truth entry " << entry << "\n";
          #endif

          for(const auto& compat: groupedUnivs)
          {
            auto& event = *compat.front(); //All compatible universes pass the same cuts
            for(auto univ: compat) univ->SetEntry(entry);

            if(::requireAll(truthPhaseSpace, event) && ::requireAll(truthSignal, event))
            {
              signal->truth(compat);
            } //If event passes all truth cuts
          } //For each error band
        } //For each entry in Truth tree
      } //If isThisJobMC
      else
      {
        //Data loop
        //TODO: This data loop is looking worse and worse as I optimize the MC loop.
        //      I'm also getting unfilled histograms from my MC jobs now.  I think
        //      it's time to put the data loop back in its own program.
        const size_t nEntries = anaTuple.GetEntries();
        auto& cv = universes["cv"].front(); //TODO: assert() that this is true?  Put data in a different program?
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
            std::bitset<64> passedCuts;
            passedCuts.set();

            for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
            {
              if(!(*sidebandCuts[whichCut])(event)) passedCuts.set(whichCut, false);
            }

            if(passedCuts.all()) signal->data({cv});
            else
            {
              const auto foundSidebands = sidebands.find(passedCuts);
              if(foundSidebands != sidebands.end())
              {
                const auto firstSideband = std::find_if(foundSidebands->second.begin(), foundSidebands->second.end(),
                                                        [&event](const auto& sideband)
                                                        { return ::requireAll(sideband->passes, event); });
                if(firstSideband != foundSidebands->second.end()) (*firstSideband)->data({cv});
              } //If found a sideband and passed all of its reco constraints
            } //If not passed all sideband-related cuts
          } //If passed all cuts not related to sidebands
        } //For each entry in data tree
      } //If not isThisJobMC

      //I've finished with this file, so I guess I read it sucessfully.  Time to count its POT.
      pot_used += thisFilesPOT;
    } //For each AnaTuple file
  } //try-catch on whole event loop
    //histFile gets destroyed and writes its histograms here
  //If I ever want to handle ROOT warnings differently, this is
  //the place to implement that behavior.
  catch(const ROOT::warning& e)
  {
    std::cerr << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const ROOT::error& e)
  {
    std::cerr << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Got a fatal std::runtime_error while running the analysis:\n"
              << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::AnalysisError;
  }

  //Print the cut table to STDOUT
  reco::Cut::TableConfig tableSize;
  for(const auto& cut: recoCuts) cut->makeTableBigEnough(tableSize);

  reco::Cut::printTableHeader(std::cout, tableSize);
  for(const auto& cut: recoCuts) cut->printTableRow(std::cout, tableSize);

  //Final Write()s to output file
  //TODO: Put POT in a TParameter<double>

  return app::CmdLine::ExitCode::Success;
}
