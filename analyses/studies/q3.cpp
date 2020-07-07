//File: q3.cpp
//Brief: A q3 VARIABLE assuming that this is a Quasi-Elastic event.
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
#include "util/CaloCorrection.h"

#ifndef ANA_Q3_CPP
#define ANA_Q3_CPP

namespace ana
{
  //An available energy VARIABLE for the CrossSection<> templates.
  struct q3
  {
    q3(const YAML::Node& config): fCaloSpline(config["caloFile"].as<std::string>("$MPARAMFILESROOT/data/Calibrations/energy_calib/CalorimetryTunings.txt"), config["caloTune"].as<std::string>()) {}

    inline std::string name() const { return "q_3"; }

    GeV truth(const evt::CVUniverse& event) const
    {
      return event.GetTruthQ3(); 
    }

    GeV reco(const evt::CVUniverse& event) const
    {
      const auto q0 = fCaloSpline.eCorrection(event.GetRecoilE() + event.GetODEnergy());
      const auto Enu = event.GetMuonP().E() + q0;
      using GeV2 = units::detail::do_pow<2, GeV>::result_t;
      const GeV2 qSquared = 2.0 * (Enu * (event.GetMuonP().E() - event.GetMuonP().z()) - pow<2>(105.7_MeV)).in<GeV2>();
      return sqrt(pow<2>(q0) + qSquared);
    }

    private:
      util::CaloCorrection fCaloSpline;
  };
}

namespace
{
  /*static ana::CrossSectionSignal<ana::q3>::Registrar q3Signal_reg("q3Signal");
  static ana::CrossSectionSideband<ana::q3>::Registrar q3Sideband_reg("q3Sideband");*/
  static ana::Resolution<ana::q3>::Registrar q3Resolution_reg("q3Resolution");
}

#endif //ANA_Q3_CPP
