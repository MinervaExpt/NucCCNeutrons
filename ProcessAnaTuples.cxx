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

//local includes
#include "gitVersion.h"

//fiducials includes
#include "fiducials/Fiducial.h"

//utility includes
#include "util/Factory.cpp"
#include "util/Table.h"
#include "util/StreamRedirection.h"
#include "util/SafeROOTName.h"

//analysis includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//Cuts includes
#include "cuts/truth/Cut.h"
#include "cuts/reco/Cut.h"
#include "PlotUtils/Cutter.h"

//models includes
#include "models/Model.h"

//TODO: This eventually replaces models
//reweighters includes
#include "reweighters/RegisterReweighters.h"
#include "systematics/RegisterVerticalSystematics.h"
#include "PlotUtils/Model.h"

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
#include "TParameter.h"

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
#include <fstream>

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

  template <class BASE, class DERIVED>
  std::vector<std::unique_ptr<BASE>> toBase(std::vector<std::unique_ptr<DERIVED>>&& derived)
  {
    std::vector<std::unique_ptr<BASE>> result;
    for(auto& ptr: derived) result.emplace_back(std::move(ptr));
    return result;
  }

  std::unique_ptr<ana::Background> null(nullptr); //TODO: This is a horrible hack so I can hold a reference to a nullptr

  template <class ITERATOR>
  auto derefOrNull(const ITERATOR it, const ITERATOR end) -> decltype(*it)
  {
    if(it == end) return ::null;
    return *it;
  }

  template <class CUT, class ...ARGS>
  bool requireAll(const std::vector<std::unique_ptr<CUT>>& cuts, const evt::Universe& event, ARGS... args)
  {
    return std::all_of(cuts.begin(), cuts.end(),
                       [&event, args...](const auto& cut)
                       {
                         return cut->passes(event, args...);
                       });
  }

  /*events getWeight(const std::vector<std::unique_ptr<model::Model>>& models, const evt::Universe& event)
  {
    return std::accumulate(models.begin(), models.end(), 1., [&event](const double product, const auto& model) { return product * model->GetWeight(event).template in<events>(); });
  }*/

  //Given a cut map, passedCuts, find the Study that this event fits in.
  ana::Study* findSelectedOrSideband(const std::bitset<64> passedCuts, fid::Fiducial& fid, evt::Universe& univ)
  {
    if(passedCuts.all()) return fid.study.get();
    else if(!passedCuts.none())
    {
      const auto foundSidebands = fid.sidebands.find(passedCuts);
      if(foundSidebands != fid.sidebands.end())
      {
        const auto firstSideband = std::find_if(foundSidebands->second.begin(), foundSidebands->second.end(),
                                                [&univ](const auto& sideband)
                                                { return sideband->passesCuts(univ); });
        if(firstSideband != foundSidebands->second.end())
        {
          return firstSideband->get();
        }
      } //If found a sideband and passed all of its reco constraints
    } //If passed all cuts not related to sidebands
    return nullptr;
  }
}

int main(const int argc, const char** argv)
{
  #ifndef NCINTEX
  ROOT::Cintex::Cintex::Enable(); //Needed to look up dictionaries for PlotUtils classes like MnvH1D
  #endif

  TH1::AddDirectory(kFALSE); //Needed so that MnvH1D gets to clean up its own MnvLatErrorBands (which are TH1Ds).

  //Components I need for the event loop
  std::vector<std::vector<evt::Universe*>> groupedUnivs;
  evt::Universe* cv;
  //std::vector<std::unique_ptr<model::Model>> reweighters;
  std::vector<std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>> reweighters;
  std::vector<std::unique_ptr<fid::Fiducial>> fiducials;
  std::string anaTupleName;

  //TODO: Move these parameters somehwere that can be shared between applications?
  std::unique_ptr<app::CmdLine> options;

  try
  {
    options.reset(new app::CmdLine(argc, argv)); //Parses the command line for input and configuration file, assembles a
                                                 //list of files to process, prepares a file for histograms, and puts the configuration
                                                 //file together.  See CmdLine.h for more details.

    //Name of the AnaTuple to read
    anaTupleName = options->ConfigFile()["app"]["AnaTupleName"].as<std::string>("NucCCNeutron");
    const bool overrideTruthCuts = options->ConfigFile()["app"]["overrideTruthCuts"].as<bool>(false);

    //MnvHadronReweight needs a TreeWrapper because it tries to connect to the tree as soon as it is created.
    //TODO: Lots of error checking :(
    if(options->TupleFileNames().empty()) throw std::runtime_error("You must pass at least one tuple file for finding branch names for MnvHadronReweight.");
    PlotUtils::ChainWrapper exampleTuple(anaTupleName.c_str());
    exampleTuple.Add(options->TupleFileNames().front());

    /*std::unique_ptr<TFile> firstTupleFile(TFile::Open(options->TupleFileNames().front().c_str(), "READ"));
    if(!firstTupleFile) throw std::runtime_error("Could not open the first tuple file, " + options->TupleFileNames().front() + ", for finding branches that MnvHadronReweight needs.");

    auto exampleRecoTree = dynamic_cast<TTree*>(firstTupleFile->Get(anaTupleName.c_str()));
    if(exampleRecoTree == nullptr) throw std::runtime_error("There is no TTree named " + anaTupleName + " in " + options->TupleFileNames().front() + ".");
    PlotUtils::TreeWrapper exampleTuple(exampleRecoTree);*/

    auto universes = app::getSystematics(&exampleTuple, *options, options->isMC());

    //Send whatever noise PlotUtils makes during setup to a file in the current working directory
    #ifdef NDEBUG
      util::StreamRedirection silencePlotUtils(std::cout, "NSFNoise.txt");
    #endif

    //The file where I will put histrograms I produce.
    util::Directory histDir(*options->HistFile);
    options->HistFile->cd();

    //Assemble Fiducials
    auto& fiducialFactory = plgn::Factory<fid::Fiducial>::instance();
    for(auto& config: options->ConfigFile()["fiducials"])
    {
      auto dirForFid = histDir.mkdir(config.first.as<std::string>());

      auto fid = fiducialFactory.Get(config.second);
      fid->name = config.first.as<std::string>();
      fid->backgrounds = app::setupBackgrounds(options->ConfigFile()["backgrounds"]);

      //N.B.: There's a technical reason why it's really hard to use util::Directory for a TParameter.
      auto nNucleons = new TParameter<double>((config.first.as<std::string>() + "_FiducialNucleons").c_str(), fid->NNucleons(options->isMC()));
      nNucleons->SetMergeMode('f'); //Use the first value -> don't add them
      nNucleons->Write();

      try
      {
        fid->study = app::setupSignal(options->ConfigFile()["signal"], dirForFid, fid->backgrounds, universes);
      } 
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up the signal Study:\n") + e.what());
      }

      PlotUtils::constraints_t<evt::Universe> truthPhaseSpace, truthSignal;
      PlotUtils::cuts_t<evt::Universe> recoCuts;

      try
      {
        truthPhaseSpace = app::setupTruthConstraints(options->ConfigFile()["cuts"]["truth"]["phaseSpace"]);
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a phase space constraint:\n") + e.what());
      }

      try
      {
        truthSignal = app::setupTruthConstraints(options->ConfigFile()["cuts"]["truth"]["signal"]);
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a signal definition constraint:\n") + e.what());
      }

      try
      {
        recoCuts = app::setupRecoCuts(options->ConfigFile()["cuts"]["reco"]);
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a reco Cut:\n") + e.what());
      }

      //Merge with Fiducial-specific Cuts
      for(auto constraint: fid->phaseSpace) truthPhaseSpace.emplace(truthPhaseSpace.begin(), constraint);
      for(auto sig: fid->signalDef) truthSignal.emplace(truthSignal.begin(), sig);
      for(auto cut: fid->recoCuts) recoCuts.emplace(recoCuts.begin(), cut);

      if(overrideTruthCuts)
      {
        truthSignal.clear();
        truthPhaseSpace.clear();
      }

      decltype(recoCuts) sidebandCuts;
      fid->sidebands = app::setupSidebands(options->ConfigFile()["sidebands"], dirForFid, fid->backgrounds, universes, recoCuts, sidebandCuts);

      fid->selection.reset(new PlotUtils::Cutter<evt::Universe, PlotUtils::detail::empty>(std::move(recoCuts), std::move(sidebandCuts), std::move(truthSignal), std::move(truthPhaseSpace)));

      fiducials.push_back(std::move(fid));
    }

    cv = universes["cv"].front();
    groupedUnivs = app::groupCompatibleUniverses(universes);
    reweighters = app::setupReweighters(options->ConfigFile()["model"]); //This MUST come after setting up universes because of the static variables that DefaultUniverse relies on
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << e.what() << "\n";
    return app::CmdLine::YAMLError;
  }

  PlotUtils::Model<evt::Universe> cvModel(std::move(reweighters));

  //End the job and warn the user if there are no Fiducials to process.
  if(fiducials.empty())
  {
    std::cerr << "No fiducials to process.  Write a \"fiducials\" block in your YAML file and try again.\n";
    return app::CmdLine::YAMLError;
  }

  const bool anyoneWantsTruth = std::any_of(fiducials.begin(), fiducials.end(), [](const auto& fid) { return fid->study->wantsTruthLoop(); });

  //Accumulate POT from each good file
  double pot_used = 0;

  //Loop over files
  LOG_DEBUG("Beginning loop over files.")
  try
  {
    for(const auto& fName: options->TupleFileNames())
    {
      LOG_DEBUG("Loading " << fName)
      //Sanity checks on AnaTuple files
      double thisFilesPOT = 0;
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
      thisFilesPOT = meta.GetValue("POT_Used", 0);
  
      auto recoTree = dynamic_cast<TTree*>(tupleFile->Get(anaTupleName.c_str()));
      if(!recoTree)
      {
        std::cerr << "Failed to find an AnaTuple named " << anaTupleName
                  << " in " << fName << ".  Skipping this file name.\n";
        continue; //TODO: Don't use continue if I can help it
      }
      //recoTree->SetCacheSize(1e7); //Read 10MB at a time
                                   //TODO: I'll bet I could make this 4k blocks or something to be more efficient on modern SSDs

      PlotUtils::TreeWrapper anaTuple(recoTree);

      //Bookkeeping for when there's no efficiency numerator
      const size_t nEntries = anaTuple.GetEntries();

      //On to the event loops
      if(options->isMC())
      {
        //MC reco loop
        const size_t nMCEntries = anaTuple.GetEntries();
        weight_hadron<PlotUtils::TreeWrapper*>(&anaTuple).setDataTree(anaTuple.GetTree());
        for(auto& compat: groupedUnivs)
        {
          for(auto& univ: compat) univ->SetTreeMC(&anaTuple);
        }

        //Get MINOS weights
        PlotUtils::MinervaUniverse::SetTruth(false);

        for(size_t entry = 0; entry < nMCEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with MC entry " << entry << "\n";
          #endif

          for(auto& fid: fiducials)
          {
            cv->SetEntry(entry);

            //Fill "fake data" by treating MC exactly like data but using a weight.
            //This is useful for closure tests and warping studies.
            PlotUtils::detail::empty CVShared;
            cvModel.SetEntry(*cv, CVShared);
            const double cvWeight = cvModel.GetWeight(*cv, CVShared);
            const auto CVPassedReco = fid->selection->isMCSelectedCV(*cv, CVShared, cvWeight);
            const auto CVStudy = findSelectedOrSideband(CVPassedReco, *fid, *cv);
            if(CVStudy) CVStudy->data(*cv, cvWeight);

            for(const auto& compat: groupedUnivs)
            {
              auto& event = *compat.front(); //All compatible universes pass the same cuts
	      PlotUtils::detail::empty shared;
	      for(const auto univ: compat) univ->SetEntry(entry); //I still need to GetWeight() for entry

              //Bitfields encoding which reco cuts I passed.  Effectively, this hashes sidebands in a way that works even
              //for sidebands defined by multiple cuts.
              const auto passedReco = fid->selection->isSelectedWithNoStats(compat, shared);

              //All compatible universes are in the same selected/sideband region because they pass the same Cuts
              auto whichStudy = findSelectedOrSideband(passedReco, *fid, *compat.front()); 
              if(whichStudy)
              {
                //Categorize by whether this is signal or some background
                if(fid->selection->isSignal(event)) whichStudy->mcSignal(compat, cvModel, shared); //for(const auto univ: compat) whichStudy->mcSignal(*univ, cvModel.GetWeight(*univ, shared));
                else //If not truthSignal
                {
                  const auto foundBackground = std::find_if(fid->backgrounds.begin(), fid->backgrounds.end(),
                                                            [&event](const auto& background)
                                                            { return ::requireAll(background->passes, event); });

                  whichStudy->mcBackground(compat, ::derefOrNull(foundBackground, fid->backgrounds.end()), cvModel, shared);
                } //If not truthSignal
              } //If found a Study to fill.  Could be either signal or sideband.  Means that at least some cuts passed.
            } //For each Fiducial
          } //For each error band
        } //For each entry in the MC tree

        //Truth loop
        if(anyoneWantsTruth)
        {
          auto truthTree = dynamic_cast<TTree*>(tupleFile->Get("Truth"));
          if(!truthTree)
          {
            std::cerr << "Failed to find an AnaTuple named Truth "
                      << " in " << fName << ".  Skipping this file name.\n";
            continue; //TODO: Don't use continue if I can help it
          }
          //truthTree->SetCacheSize(1e7); //Read 10MB at a time
          PlotUtils::TreeWrapper truthTuple(truthTree);

          const size_t nTruthEntries = truthTuple.GetEntries();
          weight_hadron<PlotUtils::TreeWrapper*>(&truthTuple).setDataTree(truthTuple.GetTree());
          for(auto& compat: groupedUnivs)
          {
            for(auto univ: compat) univ->SetTreeMC(&truthTuple); //TODO: Is MnvHadronReweight even compatible with the truth tree?
          }

          //Don't try to get MINOS weights in the truth tree loop
          PlotUtils::MinervaUniverse::SetTruth(true);

          for(size_t entry = 0; entry < nTruthEntries; ++entry)
          {
            #ifndef NDEBUG
              if((entry % printFreq) == 0) std::cout << "Done with truth entry " << entry << "\n";
            #endif

            for(auto& fid: fiducials)
            {
              cv->SetEntry(entry);
              PlotUtils::detail::empty shared;
              cvModel.SetEntry(*cv, shared);
              const double truthCVWeightForCuts = cvModel.GetWeight(*cv, shared);

              for(const auto& compat: groupedUnivs)
              {
                auto& event = *compat.front(); //All compatible universes pass the same cuts
                for(auto univ: compat) univ->SetEntry(entry); //TODO: This could be moved to the loop over compatible universes below.
  
                if(fid->selection->isEfficiencyDenom(event, truthCVWeightForCuts))
                {
                  fid->study->truth(compat, cvModel, shared);
                } //If event passes all truth cuts
              } //For each error band
            } //For each Fiducial
          } //For each entry in Truth tree
        } //If wantsTruthLoop
      } //If isThisJobMC
      else
      {
        //Data loop
        cv->SetTree(&anaTuple);

        for(size_t entry = 0; entry < nEntries; ++entry)
        {
          #ifndef NDEBUG
            if((entry % printFreq) == 0) std::cout << "Done with data entry " << entry << "\n";
          #endif

          cv->SetEntry(entry);

          PlotUtils::detail::empty shared;

          for(auto& fid: fiducials)
          {
            const auto passedCuts = fid->selection->isDataSelected(*cv, shared);
            auto whichStudy = findSelectedOrSideband(passedCuts, *fid, *cv);
            if(whichStudy) whichStudy->data(*cv);
          } //For each Fiducial
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
    std::cerr << e.what() << "\nInterrupting the event loop, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const ROOT::error& e)
  {
    std::cerr << e.what() << "\nInterrupting the event loop, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Got a fatal std::runtime_error while running the analysis:\n"
              << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::AnalysisError;
  }

  //Give Studies a chance to syncCVHistos()
  try
  {
    for(auto& fid: fiducials)
    {
      const events totalPassedCuts = fid->selection->totalWeightPassed();

      fid->study->afterAllFiles(totalPassedCuts);
      for(auto& cutGroup: fid->sidebands)
      {
        for(auto& sideband: cutGroup.second) sideband->afterAllFiles(totalPassedCuts);
      }
    }
  }
  catch(const ROOT::warning& e)
  {
    std::cerr << e.what() << "\nInterrupting afterAllFiles(), so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const ROOT::error& e)
  {
    std::cerr << e.what() << "\nInterrupting afterAllFiles(), so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::IOError;
  }
  catch(const std::runtime_error& e)
  {
    std::cerr << "Got a fatal std::runtime_error while running afterAllFiles():\n"
              << e.what() << "\nExiting immediately, so you probably got incomplete results!\n";
    return app::CmdLine::ExitCode::AnalysisError;
  }

  //Print the cut table for the first Fiducial to STDOUT
  assert(fiducials.size() > 0 && "No Fiducials to print at the end of the event loop!");
  std::cout << "#" << pot_used << " POT\n" << fiducials.front()->name << "\n#Selection:\n" << *fiducials.front()->selection << "\n\n";
  std::cout << "#Git commit hash: " << git::commitHash() << "\n";

  for(const auto& fid: fiducials)
  {
    std::string tableName = options->HistFile->GetName();
    tableName = tableName.substr(0, tableName.find('.'));
    tableName += util::SafeROOTName(fid->name);
    tableName += ".md"; //Markdown
    std::ofstream tableFile(tableName);

    tableFile << "#" << fid->name << "\n";
    tableFile << "#" << options->playlist() << "\n";
    tableFile << "#" << pot_used << " POT\n";

    tableFile << "#Selection:\n" << *fid->selection << "\n";
  }

  //Write metadata to output file
  options->HistFile->cd();
  auto pot = new TParameter<double>("POTUsed", pot_used);
  pot->Write();

  auto commitHash = new TNamed("NucCCNeutronsGitCommitHash", git::commitHash());
  commitHash->Write();

  auto playlist = new TNamed("playlist", options->playlist().c_str());
  playlist->Write();

  return app::CmdLine::ExitCode::Success;
}
