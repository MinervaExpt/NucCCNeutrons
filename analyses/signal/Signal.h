//File: Signal.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef SIG_SIGNAL_CPP
#define SIG_SIGNAL_CPP

//utilities includes
#include "util/Directory.h"

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

namespace evt
{
  class CVUniverse;
}

namespace sig
{
  class Signal
  {
    public:
      Signal(const YAML::Node& /*config*/, util::Directory& /*dir*/, std::vector<evt::CVUniverse*>& /*universes*/);
      virtual ~Signal() = default;

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.
      virtual void mc(const evt::CVUniverse& event) = 0;
      virtual void truth(const evt::CVUniverse& event) = 0;
      virtual void data(const evt::CVUniverse& event) = 0;
  };
}

#endif //SIG_SIGNAL_CPP
