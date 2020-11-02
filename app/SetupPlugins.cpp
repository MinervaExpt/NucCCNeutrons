//File: SetupPlugins.cpp
//Brief: Functions to setup Cut and Study plugins for an event loop.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//app includes
#include "app/SetupPlugins.h"

//ana includes
#include "analyses/base/Background.h"

//cuts includes
#include "cuts/reco/Cut.h"
#include "cuts/truth/Cut.h"

//models includes
#include "models/Model.h"

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/Directory.h"
#include "util/Factory.cpp"

//app includes
#include "app/CmdLine.h"

//PlotUtils includes
#include "PlotUtils/ChainWrapper.h"
#include "PlotUtils/GenieSystematics.h"
#include "PlotUtils/FluxSystematics.h"
#include "PlotUtils/MnvTuneSystematics.h"
#include "PlotUtils/AngleSystematics.h"
#include "PlotUtils/MinosEfficiencySystematics.h"
#include "PlotUtils/MuonResolutionSystematics.h"
#include "PlotUtils/MuonSystematics.h"
#include "PlotUtils/ResponseSystematics.h"

//c++ includes
#include <algorithm>

namespace
{
  //Merge entries into a map<string, vector<>>.  Normally, map<>::insert() does nothing if the key used already exists.
  //Since I'm dealing with a map<string, vector<>>, just merge with the existing vector<> if the string already exists.
  //To better match what Ben's example for the MasterAnaMacro does, I'm going to map universes to their ShortName()s.
  //This means that the standard that the user sees is the same as the error band names that appear in an output MnvH1D.
  std::map<std::string, std::vector<evt::CVUniverse*>>& merge(const std::map<std::string, std::vector<evt::CVUniverse*>>& from, std::map<std::string, std::vector<evt::CVUniverse*>>& to)
  {
    for(const auto& group: from)
    {
      const auto name = group.second.front()->ShortName();
      to[name].insert(to[name].end(), group.second.begin(), group.second.end());
    }

    return to;
  }

  //Given the names of cuts for a sideband, all reco cuts, and the cuts already associated with sidebands,
  //create a hash code that describes which cuts an event must fail to be
  //in this sideband and move any needed cuts from allCuts to sidebandCuts.
  std::bitset<64> hashCuts(const std::vector<std::string>& cutNames, std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>>& allCuts, std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>>& sidebandCuts)
  {
    std::bitset<64> result;
    result.set(); //Sets result to all true
                                                                                                                                                                              
    for(const auto& name: cutNames)
    {
      const auto found = std::find_if(allCuts.begin(), allCuts.end(), [&name](const auto& cut) { return cut->getName() == name; });
      if(found != allCuts.end())
      {
        result.set(sidebandCuts.size(), false); //BEFORE the update because of 0-based indexing
        sidebandCuts.push_back(std::move(*found));
        allCuts.erase(found);
      }
      else
      {
        const auto inSideband = std::find_if(sidebandCuts.begin(), sidebandCuts.end(), [&name](const auto& cut) { return cut->getName() == name; });
        if(inSideband != sidebandCuts.end())
        {
          result.set(std::distance(sidebandCuts.begin(), inSideband), false);
        } //If in sidebandCuts
        else throw std::runtime_error("Failed to find a cut named " + name + " for a sideband.\n");
      } //If not in allCuts
    } //For each cut name
                                                                                                                                                                              
    return result;
  }

  //An exhaustive list of all error bands in the NSF
  std::map<std::string, std::vector<evt::CVUniverse*>> getStandardSystematics(PlotUtils::ChainWrapper* chain)
  {
    //TODO: Keep this list up to date
    auto allErrorBands = PlotUtils::GetAngleSystematicsMap<evt::CVUniverse>(chain); //TODO: What if I want to change beam angle uncertainties?
    ::merge(PlotUtils::GetFluxSystematicsMap<evt::CVUniverse>(chain, PlotUtils::DefaultCVUniverse::GetNFluxUniverses()), allErrorBands);
    ::merge(PlotUtils::GetGenieSystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetMinosEfficiencySystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::Get2p2hSystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetRPASystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetLowQ2PiSystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetMuonResolutionSystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetMinervaMuonSystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetMinosMuonSystematicsMap<evt::CVUniverse>(chain), allErrorBands);
    ::merge(PlotUtils::GetResponseSystematicsMap<evt::CVUniverse>(chain, true), allErrorBands); //TODO: flag to turn off neutron response?

    return allErrorBands;
  }

  //Pick systematic universes based on a vector<string>.  A curated list of
  //all systematic universes in the NSF.
  std::map<std::string, std::vector<evt::CVUniverse*>> chooseSystematics(const std::vector<std::string>& bandsToChoose, PlotUtils::ChainWrapper* chain)
  {
    //First, make an exhaustive list of standard error bands in the NSF
    //TODO: Keep this list up to date
    auto allErrorBands  = getStandardSystematics(chain); //TODO: 

    decltype(allErrorBands) result;
    for(const auto& band: bandsToChoose)
    {
      try
      {
        result[band] = allErrorBands.at(band);
        #ifndef NDEBUG
          for(const auto& univ: result[band]) assert(univ != nullptr && "Found an error band, but it was a nullptr!");
        #endif //NDEBUG
      }
      catch(const std::out_of_range& err)
      {
        throw std::runtime_error(std::string("When selecting systematic universes to use:\n") + band + ": no such universe.\n");
      }
    }

    return result;
  }
}

namespace app
{
  std::unique_ptr<ana::Study> setupSignal(const YAML::Node& config, util::Directory& histDir,
                                          std::vector<std::unique_ptr<ana::Background>>& backgrounds,
                                          std::map<std::string, std::vector<evt::CVUniverse*>>& universes)
  {
    auto& studyFactory = plgn::Factory<ana::Study, util::Directory&, ana::Study::cuts_t&&, std::vector<std::unique_ptr<ana::Background>>&, std::map<std::string, std::vector<evt::CVUniverse*>>&>::instance();
    auto signalDir = histDir.mkdir(config["name"].as<std::string>());
    ana::Study::cuts_t noCuts = {};

    //Warn the user if there were additional Cuts specified for the signal.
    //Put those cuts in recoCuts instead.
    if(config["passes"])
    {
      std::cerr << "Warning: You specified additional cuts for the signal Study:\n" << config["passes"]
                << "\n\nPut them in reco/cuts instead!  Ignoring these Cuts.\n";
    }

    return studyFactory.Get(config, signalDir, std::move(noCuts), backgrounds, universes);
  }

  std::unordered_map<std::bitset<64>, std::vector<std::unique_ptr<ana::Study>>> setupSidebands(const YAML::Node& sidebands, util::Directory& histDir,
                                                                                               std::vector<std::unique_ptr<ana::Background>>& backgrounds,
                                                                                               std::map<std::string, std::vector<evt::CVUniverse*>>& universes,
                                                                                               std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>>& recoCuts,
                                                                                               std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>>& sidebandCuts)
  {
    auto& studyFactory = plgn::Factory<ana::Study, util::Directory&, ana::Study::cuts_t&&, std::vector<std::unique_ptr<ana::Background>>&, std::map<std::string, std::vector<evt::CVUniverse*>>&>::instance();
    auto& cutFactory = plgn::Factory<reco::Cut, std::string&>::instance();
    std::unordered_map<std::bitset<64>, std::vector<std::unique_ptr<ana::Study>>> result;

    for(auto sideband: sidebands)
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
            throw std::runtime_error(std::string("Failed to set up a reco::Cut named " + name + " for Sideband "
                                     + sideband.first.as<std::string>() + ":\n") + e.what());
          }
        }
                                                                                                                                                                                                               
        const auto hashPattern = ::hashCuts(fails, recoCuts, sidebandCuts); //Also transfers relevant cuts from recoCuts to sidebandCuts
        result[hashPattern].emplace_back(studyFactory.Get(sideband.second, dir, std::move(passes), backgrounds, universes));
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a Sideband named " + sideband.first.as<std::string>() + ":\n") + e.what());
      }
    }

    return result;
  }

  std::vector<std::unique_ptr<ana::Background>> setupBackgrounds(const YAML::Node& config)
  {
    std::vector<std::unique_ptr<ana::Background>> backgrounds;
    for(const auto background: config)
    {
      try
      {
        backgrounds.emplace_back(new ana::Background(background.first.as<std::string>(), background.second));
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a Background named " + background.first.as<std::string>() + ":\n") + e.what());
      }
    }

    return backgrounds;
  }

  std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>> setupRecoCuts(const YAML::Node& config)
  {
    std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>> recoCuts;
    auto& cutFactory = plgn::Factory<reco::Cut, std::string&>::instance();
    for(auto cut: config)
    {
      auto name = cut.first.as<std::string>();
      try
      {
        recoCuts.emplace_back(cutFactory.Get(cut.second, name));
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a reco::Cut named " + name + ":\n") + e.what());
      }
    }

    return recoCuts;
  }

  std::vector<std::unique_ptr<PlotUtils::SignalConstraint<evt::CVUniverse>>> setupTruthConstraints(const YAML::Node& config)
  {
    std::vector<std::unique_ptr<PlotUtils::SignalConstraint<evt::CVUniverse>>> constraints;
    auto& constraintFactory = plgn::Factory<truth::Cut, std::string&>::instance();
    for(auto constrain: config)
    {
      auto name = constrain.first.as<std::string>();
      try
      {
        constraints.emplace_back(constraintFactory.Get(constrain.second, name));
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a PlotUtils::SignalConstraint named " + name + ":\n") + e.what());
      }
    }

    return constraints;
  }

  std::map<std::string, std::vector<evt::CVUniverse*>> getSystematics(PlotUtils::ChainWrapper* chw, const app::CmdLine& options, const bool isMC)
  {
    std::map<std::string, std::vector<evt::CVUniverse*>> result;
    result["cv"].push_back(new evt::CVUniverse(chw));

    //"global" configuration for all DefaultCVUniverses
    //TODO: Maybe rename the "app" YAML block "physics"?  But everything in the YAML file is physics!  It still needs a better name.
    DefaultCVUniverse::SetPlaylist(options.playlist());
    DefaultCVUniverse::SetAnalysisNuPDG(DefaultCVUniverse::isFHC()?14:-14);
    DefaultCVUniverse::SetNuEConstraint(options.ConfigFile()["app"]["useNuEConstraint"].as<bool>()); //No nu-e constraint for antineutrino mode yet

    //"global" configuration for algorithms specific to my analysis
    evt::CVUniverse::SetBlobAlg(options.ConfigFile()["blobAlg"].as<std::string>("mergedTejinBlobs"));

    if(isMC)
    {
      DefaultCVUniverse::SetNFluxUniverses(options.ConfigFile()["app"]["nFluxUniverses"].as<int>());

      const auto errorBands = ::chooseSystematics(options.ConfigFile()["systematics"].as<std::vector<std::string>>(), chw);
      result.insert(errorBands.begin(), errorBands.end());
    }

    //Name of the NeutrinoInt from which to extract kinematic quantities
    const auto hypothesisName = options.ConfigFile()["app"]["HypothesisName"].as<std::string>("CCNeutrons");
    for(auto& band: result)
    {
      for(auto& univ: band.second) univ->SetHypothesisName(hypothesisName);
    }

    return result;
  }

  //Some systematic universes return true from IsVerticalOnly().  Those universes are guaranteed by the NS Framework to
  //return the same physics variables as the CV EXCEPT FOR GETWEIGHT().  So, group them together to save on
  //evaluating cuts and physics variables.
  std::vector<std::vector<evt::CVUniverse*>> groupCompatibleUniverses(const std::map<std::string, std::vector<evt::CVUniverse*>> bands)
  {
    std::vector<std::vector<evt::CVUniverse*>> groupedUnivs;
    std::vector<evt::CVUniverse*> vertical;

    for(const auto& band: bands)
    {
      if(band.first == "cv") vertical.insert(vertical.begin(), band.second.begin(), band.second.end());
      else
      {
        for(const auto univ: band.second)
        {
          if(univ->IsVerticalOnly()) vertical.push_back(univ);
          else groupedUnivs.push_back(std::vector<evt::CVUniverse*>{univ});
        }
      }
    }

    groupedUnivs.insert(groupedUnivs.begin(), vertical);

    return groupedUnivs;
  }

  std::vector<std::unique_ptr<model::Model>> setupModels(const YAML::Node& config)
  {
    if(config.size() == 0)
    {
      std::cerr << "You didn't set up any models!  There's one non-physical"
                << "case where you might want to do this: if you are "
                << "migrating from a macro that didn't do reweighting.  "
                << "I'm going to assume that's what you're doing and keep "
                << "going with a weight of 1 for every event.\n"
                << "YOU HAVE BEEN WARNED!\n";
      DefaultCVUniverse::SetNonResPiReweight(false); //I have to give this a default value if the GENIE reweight isn't set up.  It's an error to run without setting up either a GENIE reweight or no reweights at all.
    }

    std::vector<std::unique_ptr<model::Model>> models;
    auto& modelFactory = plgn::Factory<model::Model>::instance();

    for(const auto& model: config)
    {
      try
      {
        #ifndef NDEBUG
          std::cout << "Using a model of type " << model.second.Tag() << "\n";
        #endif //NDEBUG
        models.emplace_back(modelFactory.Get(model.second));
      }
      catch(const std::runtime_error& e)
      {
        throw std::runtime_error(std::string("Failed to set up a Model named " + model.first.as<std::string>() + ":\n") + e.what());
      }
    }

    #ifndef NDEBUG
      std::cout << "Using " << models.size() << " models.\n";
    #endif //NDEBUG

    return models;
  }
}
