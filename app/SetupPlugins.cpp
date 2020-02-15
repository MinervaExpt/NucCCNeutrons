//File: SetupPlugins.cpp
//Brief: Functions to setup Cut and Study plugins for an event loop.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//app includes
#include "app/SetupPlugins.h"

//ana includes
#include "analyses/base/Background.h"

//cuts includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/Directory.h"
#include "util/Factory.cpp"

//c++ includes
#include <algorithm>

namespace
{
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
                                                                                               std::vector<std::unique_ptr<reco::Cut>>& recoCuts,
                                                                                               std::vector<std::unique_ptr<reco::Cut>>& sidebandCuts)
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
}
