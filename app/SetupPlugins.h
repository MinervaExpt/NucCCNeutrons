//File: SetupPlugins.h
//Brief: Functions to setup Cut and Study plugins for an event loop.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APP_SETUPPLUGINS_H
#define APP_SETUPPLUGINS_H

//ana includes
#include "analyses/base/Study.h"

//PlotUtils includes
#include "PlotUtils/Cut.h"

//c++ includes
#include <unordered_map>
#include <bitset>

namespace ana
{
  class Background;
}

namespace util
{
  class Directory;
}

namespace PlotUtils
{
  class ChainWrapper;
}

namespace model
{
  class Model;
}

namespace app
{
  class CmdLine;

  //Set up a Study for physics in the reco signal region given a list of Backgrounds to sort by
  //and a list of systematic universes to process.
  std::unique_ptr<ana::Study> setupSignal(const YAML::Node& config, util::Directory& histDir,
                                          std::vector<std::unique_ptr<ana::Background>>& backgrounds,
                                          std::map<std::string, std::vector<evt::CVUniverse*>>& universes);

  //Set up a map from Cut hash to Study for physics in sidebands.  Moves cuts from recoCuts to
  //sidebandCuts that are related to how one or more sidebands are defined.  See hashCuts() in
  //SetupPlugins.cpp for an explanation of how the std::bitset<> key to this map works.
  std::unordered_map<std::bitset<64>, std::vector<std::unique_ptr<ana::Study>>> setupSidebands(const YAML::Node& sidebands, util::Directory& histDir,
                                                                                               std::vector<std::unique_ptr<ana::Background>>& backgrounds,
                                                                                               std::map<std::string, std::vector<evt::CVUniverse*>>& universes,
                                                                                               std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>>& recoCuts,
                                                                                               std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>>& sidebandCuts);
  //Set up Background categories.  Signal and sideband Studies will categorize histograms
  //by these Backgrounds' names.
  std::vector<std::unique_ptr<ana::Background>> setupBackgrounds(const YAML::Node& config);

  //Set up reco::Cuts that define an event selection.
  std::vector<std::unique_ptr<PlotUtils::Cut<evt::CVUniverse, PlotUtils::detail::empty>>> setupRecoCuts(const YAML::Node& config);

  //Load the set of systematics I'm going to process this event with.
  std::map<std::string, std::vector<evt::CVUniverse*>> getSystematics(PlotUtils::ChainWrapper* chw, const app::CmdLine& options, const bool isMC);

  //Group together systematic universes that always pass the same set of Cuts.  For now, that
  //just means IsVerticalOnly() universes grouped with the CV.
  std::vector<std::vector<evt::CVUniverse*>> groupCompatibleUniverses(const std::map<std::string, std::vector<evt::CVUniverse*>> bands);

  //Set up models to modify the weight of each CV event.
  std::vector<std::unique_ptr<model::Model>> setupModels(const YAML::Node& config);
}

#endif //APP_SETUPPLUGINS_H
