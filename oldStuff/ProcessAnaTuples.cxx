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
    PlotUtils::ChainWrapper tree(tupleName);

    const bool isThisJobMC = app::IsMC(options.TupleFileNames().front());
    for(const auto& fName: options.TupleFileNames())
    {
      try
      {
        const bool isThisFileMC = app::isMC(fName);
        if(isThisFileMC == isThisJobMC) tree.Add(fName);
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
      const size_t nMCEntries = tree.GetEntries();
      std::vector<evt::CVUniverse> mcUniverses = {evt::CVUniverse(blobAlg, &tree)}; //TODO: Set up full list of systematic universes

      for(size_t entry = 0; entry < nEntries; ++entry)
      {
        #ifndef NDEBUG
          if((entry % 1000) == 0) std::cout << "Done with MC entry " << entry << "\n";
        #endif

        for(const auto& event: mcUniverses)
        {
          event->SetEntry(entry);

          //TODO: MC loop
          auto analysis = MCCutTree->cut(event); //TODO: targets here needs to provide 2 things: a geometry definition and a list of analyses.
                                                 //      I need different geometry definitions for 1-Track and multi-Track samples.
          if(analysis != nullptr)
          {
            const auto whichTarget = std::find_if(analysis.targets.begin(), analysis.targets.end(),
                                                  [&event](const auto& target) { return target.isInside(event); });
            if(whichTarget != analysis.targets.end() &&  target->distanceToDivision(event))
            {
              auto& section = whichTarget->section(event);
              auto& toFill = analysis.sectionToAnalysis[section];
              if(target->isInsideTruth(event)) //Reconstructed signal
              {
                if(section.isTruthSignal(event)) toFill.Signal(event);
                else toFill.WrongMaterialBackground(event);
              }
              else //Plastic background
              {
                toFill.PlasticBackground(event);
              }
            }
            else //Plastic sideband
            {
              const auto plastic = std::find_if(analysis.plasticSlices.begin(), analysis.plasticSlices.end(),
                                                [&event](const auto& slice) { return slice.isInside(event); });
              if(plastic != analysis.plasticSlices.end()) //event could also be in the DS ECAL or DS HCAL
              {
                for(auto& target:plastic->adjacent)
                {
                  if(target->distanceToDivision(event)) analysis.sectionToAnalysis[target->section(event)].PlasticSideband(event);
                } //For each target adjacent to this slice of plastic
              } //If event is in a slice of plastic
            } //else event could be in plastic
          } //If event passed the cut tree
        } //For each MC systematic universe
      } //For each entry in the MC tree

      //Truth loop
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

          //TODO: Truth loop
          auto analysis = truthCutTree->cut(event);
          if(analysis != nullptr)
          {
            const auto whichTarget = std::find_if(analysis.targets.begin(), analysis.targets.end(),
                                                  [&event](const auto& target) { return target->isInsideTruth(event); });
            if(whichTarget != analysis.targets.end()) analysis.sectionToAnalysis[target->section(event)].Signal(event);
          } //If event passed the truthCutTree
        } //For each systematic universe
      } //For each entry in Truth tree
    } //If isThisJobMC
    else
    {
      //Data loop
      const size_t nEntries = tree->GetEntries();
      evt::CVUniverse event(blobAlg, tree);

      for(size_t entry = 0; entry < nEntries; ++entry)
      {
        #ifndef NDEBUG
          if((entry % 1000) == 0) std::cout << "Done with data entry " << entry << "\n";
        #endif

        event->setEntry(entry);

        //TODO: Data loop
        auto analysis = recoCutTree->cut(event);
        if(analysis != nullptr)
        {
          const auto whichTarget = std::find_if(analysis.targets.begin(), analysis.targets.end(),
                                                [&event](const auto& target) { return target.isInside(event); });
          if(whichTarget != analysis.targets.end() && target->distanceToDivision(event))
          {
            target->section(event).Signal(event);
          }
          else //If event is not in a target, is it in plastic?
          {
            const auto plastic = std::find_if(analysis.plasticSlices.begin(), analysis.plasticSlices.end(),
                                              [&event](const auto& slice) { return slice.isInside(event); });
            if(plastic != analysis.plasticSlices.end())
            {
              for(auto& target: plastic->adjacent)
              {
                if(target->distanceToDivision(event)) analysis.sectionToAnalysis[target->section(event)].PlasticSideband(event);
              } //For each target adjacent to this slice of plastic
            } //If event is in plastic
          } //Is event in a target?
        } //If analysis passed the recoCutTree
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
