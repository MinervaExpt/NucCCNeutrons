//File: Signal.cpp
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef SIG_SIGNAL_CPP
#define SIG_SIGNAL_CPP

namespace sig
{
  class Signal
  {
    public:
      Signal(util::Directory& /*dir*/, const YAML::Node& /*config*/) = default;
      virtual ~Signal() = default;

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.
      virtual void mc(const CVUniverse& event) = 0;
      virtual void truth(const CVUniverse& event) = 0;
      virtual void data(const CVUniverse& event) = 0;
  };
}

#endif //SIG_SIGNAL_CPP
