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

namespace bkg
{
  class Background;
}

namespace sig
{
  class Signal
  {
    protected:
      using background_t = std::unique_ptr<bkg::Background>;

    public:
      Signal(const YAML::Node& /*config*/, util::Directory& /*dir*/, std::vector<background_t>& /*backgrounds*/, std::vector<evt::CVUniverse*>& /*universes*/);
      virtual ~Signal() = default;

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.
      virtual void mcSignal(const evt::CVUniverse& event) = 0;
      virtual void mcBackground(const evt::CVUniverse& event, const background_t& background) = 0;
      virtual void truth(const evt::CVUniverse& event) = 0;
      virtual void data(const evt::CVUniverse& event) = 0;
  };
}

#endif //SIG_SIGNAL_CPP
