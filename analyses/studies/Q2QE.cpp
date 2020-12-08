//File: Q2QE.cpp
//Brief: A Q squared VARIABLE calculated assuming that an event is Quasi-Elastic.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//util includes
#include "util/units.h"
#include "util/mathWithUnits.h"

//analyses includes
/*#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"*/
#include "analyses/studies/Resolution.h"

//c++ includes
#include <string>

//evt includes
#include "evt/Universe.h"

//utility includes
#include "util/Factory.cpp"

#ifndef ANA_Q2QE_CPP
#define ANA_Q2QE_CPP

//Custom unit name for GeV^2
namespace units
{
  template <>
  struct attributes<typename detail::do_pow<2, GeV>::result_t>
  {
    static constexpr auto name = "GeV^{2}";
  };
}

namespace ana
{
  //An available energy VARIABLE for the CrossSection<> templates.
  struct Q2QE
  {
    using unit = decltype(1_GeV * 1_GeV);

    Q2QE(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "Q^{2}_{QE}"; }

    unit truth(const evt::Universe& event)
    {
      return event.GetTruthQ2(); 
    }

    unit reco(const evt::Universe& event)
    {
      //TODO: Recalculate from recoil here to pick up dependence on systematics.
      const GeV Enu = EnuQE(event);
      const auto half =  Enu * (event.GetEmu() - event.GetPMu().z) - pow<2>(muonMass);
      return half.in<unit>() * 2.0;
    }

    private:
      GeV EnuQE(const evt::Universe& event) const
      {
        const GeV hadronInMass = , hadronOutMass = , leptonMass = ;
        const auto denom = 2.0 * (hadronInMass - bindingE - event.GetEmu() + event.GetPmu().z);
        if(denom == 0) return denom;

        const auto num = pow<2>(hadronOutMass) - pow<2>(hadronInMass - bindingE) - pow<2>(leptonMass) + 2.0 * (hadronInMass - bindingE) * event.GetEmu();

        return num.in<unit>() / denom.in<GeV>();
      }
  };
}

namespace
{
  /*static ana::CrossSectionSignal<ana::Q2QE>::Registrar Q2Signal_reg("Q2Signal");
  static ana::CrossSectionSideband<ana::Q2QE>::Registrar Q2Sideband_reg("Q2Sideband");*/
  static ana::Resolution<ana::Q2QE>::Registrar Q2Resolution_reg("Q2Resolution");
}

#endif //ANA_Q2QE_CPP
