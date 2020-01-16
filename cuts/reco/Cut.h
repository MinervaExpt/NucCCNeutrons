//File: Cut.h
//Brief: A Cut decides whether a CVUniverse should be considered
//       reco signal.  Cuts can be used to define
//       Signals, Sidebands, and Backgrounds.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_CUT_H
#define RECO_CUT_H

//N.B.: Normally, I'd use forward declarations as aggressively as possible here.
//      But putting the includes here saves me A LOT of typing.

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/Factory.cpp"

namespace reco
{
  //TODO: This could easily be a class template on CVUniverse
  //      for other analyses to use it.
  class Cut
  {
    public:
      Cut(const YAML::Node& /*config*/) {}
      virtual ~Cut() = default;
      
      //Public interface.  If you're writing a new Cut, look
      //at the private implementation below.
      //
      //I designed Cut this way so I have a hook to keep
      //statistics for a cut table later.
      bool operator ()(const evt::CVUniverse& event);

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const = 0;
  };
}

#endif //RECO_CUT_H
