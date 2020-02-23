//File: EAvailable.cpp
//Brief: A EAvailable VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
/*#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"*/
#include "analyses/studies/Resolution.h"

//c++ includes
#include <string>

//evt includes
#include "evt/CVUniverse.h"

//utility includes
#include "util/Factory.cpp"

#ifndef ANA_EAVAILABLE_CPP
#define ANA_EAVAILABLE_CPP

namespace ana
{
  //An available energy VARIABLE for the CrossSection<> templates.
  struct EAvailable
  {
    EAvailable(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "E_available"; }

    GeV truth(const evt::CVUniverse& event)
    {
      return event.GetTruthEAvailable(); 
    }

    GeV reco(const evt::CVUniverse& event)
    {
      return event.GetRecoilE();
    }
  };
}

namespace
{
  /*static ana::CrossSectionSignal<ana::EAvailable>::Registrar EAvailableSignal_reg("EAvailableSignal");
  static ana::CrossSectionSideband<ana::EAvailable>::Registrar EAvailableSideband_reg("EAvailableSideband");*/
  static ana::Resolution<ana::EAvailable>::Registrar EAvailableResolution_reg("EAvailableResolution");
}

#endif //ANA_EAVAILABLE_CPP
