//File: SetupPlugins.h
//Brief: Functions to setup Cut and Study plugins for an event loop.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APP_SETUPPLUGINS_H
#define APP_SETUPPLUGINS_H

//ana includes
#include "analyses/base/Study.h"

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
  std::unique_ptr<ana::Study> setupSignal(const YAML::Node& config, util::Directory& histDir,
                                          std::vector<std::unique_ptr<ana::Background>>& backgrounds,
                                          std::map<std::string, std::vector<evt::CVUniverse*>>& universes);
}

#endif //APP_SETUPPLUGINS_H
