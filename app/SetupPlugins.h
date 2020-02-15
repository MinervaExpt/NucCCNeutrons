//File: SetupPlugins.h
//Brief: Functions to setup Cut and Study plugins for an event loop.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APP_SETUPPLUGINS_H
#define APP_SETUPPLUGINS_H

//ana includes
#include "analyses/base/Study.h"

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

namespace app
{
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
                                                                                               std::vector<std::unique_ptr<reco::Cut>>& recoCuts,
                                                                                               std::vector<std::unique_ptr<reco::Cut>>& sidebandCuts);
}

#endif //APP_SETUPPLUGINS_H
