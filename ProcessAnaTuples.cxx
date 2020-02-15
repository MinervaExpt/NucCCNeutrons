//File: ProcessAnaTuples.cxx
//Brief: A program to make plots from NucCCNeutrons NTuples.  Infers
//       whether first file is data or MC based on whether Truth tree
//       is available and then asserts that this is true for all other
//       files.  The target and analysis cuts it uses need to be configured
//       by a YAML file.
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
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//Cuts includes
#include "cuts/truth/Cut.h"
#include "cuts/reco/Cut.h"

//app includes
#include "app/CmdLine.h"
#include "app/IsMC.h"
#include "app/SetupPlugins.h"

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

  std::unique_ptr<ana::Background> null(nullptr); //TODO: This is a horrible hack so I can hold a reference to a nullptr

  template <class ITERATOR>
  auto derefOrNull(const ITERATOR it, const ITERATOR end) -> decltype(*it)
  {
    if(it == end) return ::null;
    return *it;
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

  //Components I need for the event loop
  std::vector<std::vector<evt::CVUniverse*>> groupedUnivs;
  std::vector<std::unique_ptr<ana::Background>> backgrounds;
  std::unique_ptr<ana::Study> signal;
  std::vector<std::unique_ptr<truth::Cut>> truthPhaseSpace;
  std::vector<std::unique_ptr<truth::Cut>> truthSignal;
  std::vector<std::unique_ptr<reco::Cut>> recoCuts;
  std::unordered_map<std::bitset<64>, std::vector<std::unique_ptr<ana::Study>>> sidebands;
  decltype(recoCuts) sidebandCuts;

  //TODO: Move these parameters somehwere that can be shared between applications?
  constexpr auto anaTupleName = "NucCCNeutron";
  std::unique_ptr<app::CmdLine> options;

  try
  {
    //TODO: Options' HistFile is getting deleted when options goes out of scope!
    options.reset(new app::CmdLine(argc, argv)); //Parses the command line for input and configuration file, assembles a
                                            //list of files to process, prepares a file for histograms, and puts the configuration
                                            //file together.  See CmdLine.h for more details.

    //Set up other application options
    const auto& appOptions = options->ConfigFile()["app"];
    const auto blobAlg = appOptions["BlobAlg"].as<std::string>("proximity");

    //The file where I will put histrograms I produce.
    //TODO: I learned when helping Christian that I can't use TFile::Write() to write
    //      MnvH1Ds' plots because MnvH1D insists on deleteing its own TH1Ds.  I've got
    //      to either Write() each TH1D or convince TFile not to delete the objects it
    //      owns.  I think there's a flag for the latter in TObject.
    util::Directory histDir(*options->HistFile);

    auto universes = ::getSystematics(nullptr, *options, options->isMC());
    backgrounds = app::setupBackgrounds(options->ConfigFile()["backgrounds"]);
    signal = app::setupSignal(options->ConfigFile()["signal"], histDir, backgrounds, universes);
    truthPhaseSpace = plgn::loadPlugins<truth::Cut>(options->ConfigFile()["cuts"]["truth"]["phaseSpace"]); //TODO: Tell the user which Cut failed
    truthSignal = plgn::loadPlugins<truth::Cut>(options->ConfigFile()["cuts"]["truth"]["signal"]); //TODO: Tell the user which Cut failed
    recoCuts = app::setupRecoCuts(options->ConfigFile()["cuts"]["reco"]);
    sidebands = app::setupSidebands(options->ConfigFile()["sidebands"], histDir, backgrounds, universes, recoCuts, sidebandCuts);
    groupedUnivs = ::groupCompatibleUniverses(universes);
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << e.what();
    return app::CmdLine::YAMLError;
  }

  //Accumulate POT from each good file
  double pot_used = 0;

  //Loop over files
  LOG_DEBUG("Beginning loop over files!")
  try
  {
    for(const auto& fName: options->TupleFileNames())
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
      if(app::IsMC(fName) != options->isMC())
      {
        std::cerr << "This job " << (options->isMC()?"is":"is not")
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
      if(options->isMC())
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
              ana::Study* sideband = nullptr; //TODO: Come up with some clever return value semantics to avoid the raw pointer here
              if(relevantSidebands != sidebands.end())
              {
                const auto found = std::find_if(relevantSidebands->second.begin(), relevantSidebands->second.end(),
                                                [&event](const auto& whichSideband)
                                                { return whichSideband->passesCuts(event); });
                if(found != relevantSidebands->second.end()) sideband = found->get();
              }

              //Categorize by whether this is signal or some background
              if(::requireAll(truthSignal, event))
              {
                if(passedReco.all())
                {
                  for(const auto univ: compat) signal->mcSignal(*univ);
                }
                else if(sideband) //If this is a sideband I'm interested in
                {
                  for(const auto univ: compat) sideband->mcSignal(*univ);
                }
              }
              else //If not truthSignal
              {
                const auto foundBackground = std::find_if(backgrounds.begin(), backgrounds.end(),
                                                          [&event](const auto& background)
                                                          { return ::requireAll(background->passes, event); });

                if(passedReco.all())
                {
                  for(const auto univ: compat) signal->mcBackground(*univ, ::derefOrNull(foundBackground, backgrounds.end()));
                }
                else if(sideband)
                {
                  for(const auto univ: compat) sideband->mcBackground(*univ, ::derefOrNull(foundBackground, backgrounds.end()));
                }
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
              for(const auto univ: compat) signal->truth(*univ);
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
        auto& cv = *groupedUnivs.front().front();
        assert(cv.ShortName() == "cv");
        cv.SetTree(&anaTuple);

        for(size_t entry = 0; entry < nEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with data entry " << entry << "\n";
          #endif

          cv.SetEntry(entry);
          if(::requireAll(recoCuts, cv))
          {
            std::bitset<64> passedCuts;
            passedCuts.set();

            for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
            {
              if(!(*sidebandCuts[whichCut])(cv)) passedCuts.set(whichCut, false);
            }

            if(passedCuts.all()) signal->data(cv);
            else
            {
              const auto foundSidebands = sidebands.find(passedCuts);
              if(foundSidebands != sidebands.end())
              {
                const auto firstSideband = std::find_if(foundSidebands->second.begin(), foundSidebands->second.end(),
                                                        [&cv](const auto& sideband)
                                                        { return sideband->passesCuts(cv); });
                if(firstSideband != foundSidebands->second.end()) (*firstSideband)->data(cv);
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

  //Give Studies a chance to syncCVHistos()
  signal->afterAllFiles();
  for(auto& cutGroup: sidebands)
  {
    for(auto& sideband: cutGroup.second) sideband->afterAllFiles();
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
