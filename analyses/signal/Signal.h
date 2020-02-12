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

//c++ includes
#include <map>

namespace evt
{
  class CVUniverse;
}

namespace bkg
{
  class Background;
}

namespace sig
{
  class Signal
  {
    public:
      using background_t = std::unique_ptr<bkg::Background>;

      Signal(const YAML::Node& /*config*/, util::Directory& /*dir*/, std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::CVUniverse*>>& /*universes*/);
      virtual ~Signal() = default;

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.  All universes passed to the
      //same function call are guaranteed by the event loop to
      //return the same physics values EXCEPT WEIGHT.  So, do
      //any calculations with univs.front() and then loop over
      //all univs to Fill() histograms.
      virtual void mcSignal(const std::vector<evt::CVUniverse*>& univs) = 0;
      virtual void mcBackground(const std::vector<evt::CVUniverse*>& univs, const background_t& background) = 0;
      virtual void truth(const std::vector<evt::CVUniverse*>& univs) = 0;
      virtual void data(const std::vector<evt::CVUniverse*>& univs) = 0;
  };
}

#endif //SIG_SIGNAL_CPP
