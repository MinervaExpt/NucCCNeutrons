//File: Cut.h
//Brief: A Cut decides whether a CVUniverse should be considered
//       truth signal.  Cuts can be used to define
//       Signals, Sidebands, and backgrounds.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_CUT_H
#define TRUTH_CUT_H

//N.B.: Normally, I'd use forward declarations as aggressively as possible here.
//      But putting the includes here saves me A LOT of typing.
//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//evt includes
#include "evt/CVUniverse.h"

//utilities includes
#include "util/Factory.cpp"

namespace truth
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

  template <class DERIVED>
  using Registrar = plgn::Registrar<truth::Cut, DERIVED>;
}

#endif //TRUTH_CUT_H
