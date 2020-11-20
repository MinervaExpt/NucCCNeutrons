//File: EventDisplay.h
//Brief: MINERvA has a web app called Arachne for its sole event display.  This program spits out
//       links to Arachne pages on std::clog == STDERR.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/base/Study.h"

//utilities includes
#include "util/WithUnits.h"

//evt includes
#include "evt/CVUniverse.h"
#include "evt/arachne.h"
#include "evt/EventID.h"

//c++ includes
#include <iostream>

#ifndef ANA_EVENTDISPLAY_H
#define ANA_EVENTDISPLAY_H

namespace ana
{
  class EventDisplay: public Study
  {
    public:
      //TODO: A feature to use the Rodriges instance of Arachne might be cool
      EventDisplay(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                     const std::vector<background_t>& backgrounds,
                     std::map<std::string, std::vector<evt::CVUniverse*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes)
      {
      }

      virtual void mcSignal(const evt::CVUniverse& event, const events /*weight*/) override
      {
        std::clog << util::arachne(event.GetEventID(false), false) << "\n";
      }

      //TODO: A feature to show just background events of a specific type might be cool.
      //mcBackground failed the truth signal selection.
      virtual void mcBackground(const evt::CVUniverse& /*event*/, const background_t& /*background*/, const events /*weight*/) override {}
                                                                                                                        
      //TODO: If truth-only studies ever work, I need to do some printing here
      //Truth tree with truth information.  Passes truth signal definition and phase space.
      virtual void truth(const evt::CVUniverse& /*event*/, const events /*weight*/) override {}
                                                                                                                        
      //Data AnaTuple with only reco information.  These events passed all reco Cuts. 
      //Truth branches may be in an undefined state here, so be very careful not to use them.
      virtual void data(const evt::CVUniverse& event, const events /*weight*/) override
      {
        std::clog << util::arachne(event.GetEventID(true), true) << "\n";
      }
                                                                                                                        
      //Optional function called once per job after the last file in the event loop.
      virtual void afterAllFiles(const events /*passedSelection*/) override {}

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }
  };
}

namespace
{
  static ana::Study::Registrar<ana::EventDisplay> EventDisplay_reg("EventDisplay");
}

#endif //ANA_EVENTDISPLAY_H
