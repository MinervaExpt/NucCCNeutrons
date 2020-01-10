//File: Cut.h
//Brief: A Cut decides whether a CVUniverse should be considered
//       truth signal.  Cuts can be used to define
//       Signals, Sidebands, and backgrounds.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_CUT_H
#define TRUTH_CUT_H

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace truth
{
  //TODO: This could easily be a class template on CVUniverse
  //      for other analyses to use it.
  class Cut
  {
    public:
      Cut(const YAML::Node& /*config*/) = default;
      virtual ~Cut() = default;
      
      //Public interface.  If you're writing a new Cut, look
      //at the private implementation below.
      //
      //I designed Cut this way so I have a hook to keep
      //statistics for a cut table later.
      bool operator ()(const CVUniverse& event);

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const CVUniverse& event) const = 0;
  };
}

#endif //TRUTH_CUT_H
