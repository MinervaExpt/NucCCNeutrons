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
  class Cut
  {
    public:
      Cut(const YAML::Node& /*config*/, const std::string& name): fName(name) {}
      virtual ~Cut() = default;
      
      //Public interface.  If you're writing a new Cut, look
      //at the private implementation below.
      //
      //I designed Cut this way so I have a hook to keep
      //statistics for a cut table later.
      bool operator ()(const evt::CVUniverse& event, const double weight = 1, const bool isSignal = true);

      //Access to Cut performance statistics
      inline double signalPassed() const { return fSignalPassed; }
      inline size_t totalPassed() const { return fTotalPassed; }
      inline const std::string& name() const { return fName; }

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const = 0;

    private:
      std::string fName; //Name of this cut for reporting

      //Data for the cut table
      double fSignalPassed = 0; //Sum of event weights for which operator() returned true
      double fTotalPassed = 0; //Number of times an event passed this Cut
  };
}

#endif //RECO_CUT_H
