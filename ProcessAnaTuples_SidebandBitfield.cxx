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

//geo includes
#include "geo/Target.h"

//evt includes
#include "evt/CVUniverse.h"

//utility includes
#include "util/Factory.cpp"

//analysis includes
#include "analysis/Analysis.h"

//TODO: Cuts includes

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

int main(const int argc, const char** argv)
{
  //TODO: Move these parameters somehwere that can be shared between applications?
  constexpr auto tupleName = "NucCCNeutron";

  try
  {
    const apo::CmdLine options(argc, argv); //Parses the command line for input and configuration file, assembles a
                                            //list of files to process, prepares a file for histograms, and puts the configuration
                                            //file together.  See CmdLine.h for more details.

    //TODO: Set up detector geometry.  Each target section will have its own
    //set of plots.

    //Set up other application options
    const auto& appOptions = options.ConfigFile()["app"];
    const auto blobAlg = appOptions["BlobAlg"].as<std::string>("proximity");

    //Check files to process
    PlotUtils::ChainWrapper anaTuple(tupleName);

    const bool isThisJobMC = app::IsMC(options.TupleFileNames().front());
    for(const auto& fName: options.TupleFileNames())
    {
      try
      {
        const bool isThisFileMC = app::isMC(fName);
        if(isThisFileMC == isThisJobMC) anaTuple.Add(fName);
        else
        {
          std::cerr << fName << (isThisFileMC?" is":" is not") << " an MC file, but this job "
                    << (isThisJobMC?" did":" did not") << " start with an MC file.  You can "
                    << "only process either all data or all MC files with ProcessAnaTuples.  "
                    << "This check looks for the Truth tree.\n";
          return apo::CmdLine::ExitCode::IOError;
        }
      }
      catch(const PlotUtils::ChainWrapper::BadFile& e)
      {
        std::cerr << "Failed to add an AnaTuple file to ChainWrapper: " << e.what() << "\n";
        std::cerr << "Continuing without that file.\n";
      }
    }

    if(isThisJobMC)
    {
      //MC loop
      const size_t nMCEntries = anaTuple.GetEntries();
      std::vector<evt::CVUniverse> mcUniverses = {evt::CVUniverse(blobAlg, &anaTuple)}; //TODO: Set up full list of systematic universes

      for(size_t entry = 0; entry < nEntries; ++entry)
      {
        #ifndef NDEBUG
          if((entry % 1000) == 0) std::cout << "Done with MC entry " << entry << "\n";
        #endif

        for(const auto& event: mcUniverses)
        {
          event->SetEntry(entry);

          //MC loop
          //TODO: Categories between basicCuts and sideband-related cuts?
          if(std::all_of(basicCuts.begin(), basicCuts.end(), [&event](const auto& cut) { return cut->truth(event) && cut->reco(event); }))
          {
            //Bitfields encoding which truth and reco cuts I passed.  Effectively, this hashes sidebands in a way that works even
            //for sidebands defined by multiple cuts.
            uint64_t passedTruth = 0, passedReco = 0; //TODO: What does the default value for each bit need to be to work with XOR when I have fewer than 64 cuts?
            constexpr decltype(passedTruth) passedAll = 1 << sidebandCuts.size() - 1; //TODO: Change this to somehting like 0b1 << nCuts - 1

            for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
            {
              const auto& cut = sidebandCuts[whichCut];
              passedTruth |= cut->truth(event) << whichCut;
              passedReco |= cut->reco(event) << whichCut;
            }

            if(!(passedTruth ^ passedAll) && !(passedReco ^ passedAll)) signal->mc(event); //TODO: I don't think this requires passing all cuts!
            else
            {
              const auto foundSideband = sidebands.find(passedReco); //Look up the sideband, if any, by which reco cuts failed
              const auto foundBackground = backgrounds.find(passedTruth); //Look up the background category, if any, by which truth cuts failed
              if(foundSideband != sidebands.end())
              {
                if(!(passedTruth ^ passedAll)) foundSideband->FillSignal(event);
                else (*foundSideband).FillBackground(event, foundBackground); //TODO: Make sure backgrounds.end() is the "Other" category
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
      //TODO: This means opening each file AGAIN.  Isn't that where a lot of our I/O overhead comes from in the first place?
      //      Down with ChainWrapper!  Or, I could be especially stubborn and use one ChainWrapper per AnaTuple file.  I think
      //      the best approach in the short term is to accept a TreeWrapper* in DefaultCVUniverse and friends.
      //
      //      In the very short term, I'm going to create 2 ChainWrappers per TFile and do the loop over TFiles myself.  I have
      //      yet to find a systematic universe that uses the extra TChain functionality in ChainWrapper.
      PlotUtils::ChainWrapper truthTree("Truth");
      for(const auto& fName: options.TupleFileNames()) truthTree.AddFile(fName); //I already made sure these were all truth files

      const size_t nTruthEntires = truthTree.GetEntries();
      std::vector<evt::CVUniverse> truthUniverses = {evt::CVUniverse(blobAlg, &truthTree)}; //TODO: Set up fill list of systematic universes

      for(size_t entry = 0; entry < nEntries; ++entry)
      {
        #ifndef NDEBUG
          if((entry % 1000) == 0) std::cout << "Done with truth entry " << entry << "\n";
        #endif

        for(const auto& event: truthUniverses)
        {
          event->SetEntry(entry);

          //Truth loop
          if(std::all_of(basicCuts.begin(), basicCuts.end(), [&event](const auto& cut) { return cut->truth(event); })
             && std::all_of(sidebandCuts.begin(), sidebandCuts.end(), [&event](const auto& cut) { return cut->truth(event); }))
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
          if((entry % 1000) == 0) std::cout << "Done with data entry " << entry << "\n";
        #endif

        event->setEntry(entry);
        if(std::all_of(basicCuts.begin(), basicCuts.end(), [&event](const auto& cut) { return cut->truth(event); }))
        {
          uint64_t passedCuts = 0;
          constexpr decltype(passedCuts) passedAll = 1 << sidebandCuts.size() - 1; //TODO: Is 0 an appropriate default with XOR?

          for(size_t whichCut = 0; whichCut < sidebandCuts.size(); ++whichCut)
          {
            passedCuts |= sidebandCuts[whichCut]->reco(event) << whichCut;
          }

          if(!(passedCuts ^ passedAll)) signal->FillData(event);
          else
          {
            const auto foundSideband = sidebands.find(passedCuts);
            if(foundSideband != sidebands.end()) foundSideband->FillData(event);
          } //If not passed all sideband-related cuts
        } //If passed all cuts not related to sidebands
      } //For each entry in data tree
    } //If not isThisJobMC
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
