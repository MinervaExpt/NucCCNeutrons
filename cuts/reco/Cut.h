//File: Cut.h
//Brief: A Cut decides whether a Universe should be considered
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
#include "evt/Universe.h"

//util includes
#include "util/Factory.cpp"

//PlotUtils includes
#include "PlotUtils/Cut.h"

//TODO: Don't need this once debugging is done
using PCut = PlotUtils::Cut<evt::Universe>;

namespace reco
{
  class Cut: public PCut
  {
    public:
      Cut(const YAML::Node& /*config*/, const std::string& name): PlotUtils::Cut<evt::Universe>(name) {}
      virtual ~Cut() = default;
      
      template <class DERIVED>
      using Registrar = plgn::Registrar<reco::Cut, DERIVED, std::string&>;

      //Adapter for old Cut interface
      std::string name() const { return getName(); }
  };
}

#endif //RECO_CUT_H
