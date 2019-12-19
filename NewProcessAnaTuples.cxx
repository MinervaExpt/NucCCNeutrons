//File: NewProcessAnaTuples.cxx
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
    const auto& detConfigs = options.ConfigFile()["detectors"];
    apo::Detector oneTrack(detConfigs["OneTrack"], "One Track", *options.HistFile),
                  twoTrack(detConfigs["TwoTrack"], "Two Track", *options.HistFile);

    //Set up other application options
    const auto& appOptions = options.ConfigFile()["app"];
    const auto blobAlg = appOptions["BlobAlg"].as<std::string>("proximity");

    //TODO: Set up systematic universes
    PlotUtils::ChainWrapper tree(tupleName);

    const bool isThisJobMC = app::IsMC(options.TupleFileNames().front());
    for(const auto& fName: options.TupleFileNames())
    {
      try
      {
        const bool isThisFileMC = isMC(fName);
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

    const size_t nEntries = tree.GetEntries();
    std::vector<CVUniverse> universes{CVUniverse(blobAlg, &tree)};

    //TODO: Don't loop over universes for data.
    //TODO: Special loop over Truth tree for MC.
    //TODO: Provide truth-matched information for MC.
    for(size_t entry = 0; entry < nEntries; ++entry)
    {
      #ifndef NDEBUG
        if((whichEntry % 1000) == 0) std::cout << "Done with entry " << whichEntry << "\n";
      #endif

      //TODO: Reco only loop

      for(const auto& event: universes)
      {
        event->SetEntry(entry);
        //TODO: For now, look up target.  This should provide study.
        //Process the CV
        NotCached readFromTree(schema);
        const SchemaView& cache = readFromTree; //Don't force a read to start with
        bool filledCache = false;

        //Target lookup that is specific to my analysis.  Eventually, I might want this to become
        //a "generalized Cut step".  Keep it simple for now.  All analyses can probably benefit
        //from some of this machinery anyway.
        auto& det = (cache.nTracks > 1)?twoTrack:oneTrack;
        if(det.isFiducial(cache.vtx)) //TODO: Do I gain anything by always caching the vertex?  I'd just put it in SchemaView.
        {
          //TODO: Make sure cuts are run in the order they appear in the YAML file.
          if(std::all_of(cuts.begin(), cuts.end(), [&cache](const auto& cut) { return cut(cache); })) //TODO: Caching for cuts?
          {
            auto& target = det.findTarget(cache.vtx);
            auto& section = target.first->section(cache.vtx);

            //No need to check for whether cache has been filled.  It has to be empty at this point.
            //TODO: It looks bad to use a specific Study for caching, but I'm probably going to require
            //      that all Targets use the same Study type anyway.  In any case, the physics should
            //      not be affected because all caches for a given event should provide the same numbers.
            cache = target.second->cache(schema); //Cache values once for all universes to use
            filledCache = true;

            //TODO: Per-target candidate filtering
            target.second->analyze(cache);
            section->analyze(cache);

            //Process vertical error bands.  They only provide weights and don't actually shift values.
            for(const auto& vert: verticalUniverses)
            {
              vert->setCache(cache);

              //TODO: Per-target candidate filtering
              target.second->analyze(*vert);
              study->analyze(*vert);
            } //For each vertical systematic universe
          } //If CV passes all cuts

        //Process lateral error bands.  They may pass cuts independent of the CV.
        for(const auto& lateral: lateralUniverses) //Each univ is a plugin populated from a YAML file
        {
          lateral->setCache(cache); //Use the cached values if I've already cached them anyway.
                                    //Otherwise, cuts read directly from tree.
                                    
          auto& det = (lateral->nTracks > 1)?twoTrack:oneTrack;
          if(det.isFiducial(lateral->vtx))
          {
            if(std::all_of(cuts.begin(), cuts.end(), [&lateral](const auto& cut) { return cut(*lateral); })) //TODO: Caching for cuts?
            {
              auto& target = det.findTarget(lateral->vtx);
              auto& section = target.first->section(lateral->vtx);

              //Fill univCache the first time a lateral universe passes all cuts.
              //This should reduce the amount of data read from file and thus encourage
              //the OS to cache bits of this file for rapid iteration.
              if(!filledCache)
              {
                cache = target.second->cache(schema);
                lateral->setCache(cache);
                filledCache = true;
              } //If no universe has passed all cuts so far

              target.second->analyze(*lateral);
              section.analyze(*lateral);
            } //If this univ passes all Cuts
          } //If this vertex passes a fiducial cut
        } //For each lateral systematic universe
      } //For each universe
    } //Per entry in ntuple
  } //try-catch
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
