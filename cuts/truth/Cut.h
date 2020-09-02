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

//PlotUtils includes
#include "PlotUtils/Cut.h"

namespace truth
{
  class Cut: public PlotUtils::SignalConstraint<evt::CVUniverse>
  {
    public:
      Cut(const YAML::Node& /*config*/): PlotUtils::SignalConstraint<evt::CVUniverse>("FIXME") {}
      virtual ~Cut() = default;
      
      template <class DERIVED>
      using Registrar = plgn::Registrar<truth::Cut, DERIVED>;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const = 0;

      //Forward legacy passesCut() onto what PlotUtils::SignalConstraint expects
      virtual bool checkConstraint(const evt::CVUniverse& event) const override;
  };
}

#endif //TRUTH_CUT_H
