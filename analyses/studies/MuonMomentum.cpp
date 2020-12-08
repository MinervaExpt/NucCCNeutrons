//File: MuonMomentum.cpp
//Brief: A MuonMomentum VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"
#include "analyses/studies/CrossSection2DSignal.h"

//c++ includes
#include <string>

//evt includes
#include "evt/Universe.h"

//utility includes
#include "util/Factory.cpp"

#ifndef ANA_MUONMOMENTUM_CPP
#define ANA_MUONMOMENTUM_CPP

namespace ana
{
  //A muon momentum magnitude VARIABLE for the CrossSection<> templates.
  struct MuonMomentum
  {
    MuonMomentum(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "Muon Momentum"; }

    GeV truth(const evt::Universe& event)
    {
      return event.GetTruthPmu().p().mag();
    }

    GeV reco(const evt::Universe& event)
    {
      return event.GetMuonP().p().mag();
    }
  };

  //A muon p_z VARIABLE
  struct MuonPz
  {
    MuonPz(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "Muon p_z"; }

    GeV truth(const evt::Universe& event)
    {
      return event.GetTruthPmu().z();
    }

    GeV reco(const evt::Universe& event)
    {
      return event.GetMuonP().z();
    }
  };

  //A muon p_transverse VARIABLE
  struct MuonPT
  {
    MuonPT(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "Muon p_T"; }

    GeV truth(const evt::Universe& event)
    {
      const units::XYZVector<double> zHat{0, 0, 1};
      return event.GetTruthPmu().p().cross(zHat).mag();
    }
    
    GeV reco(const evt::Universe& event)
    {
      const units::XYZVector<double> zHat{0, 0, 1};
      return event.GetMuonP().p().cross(zHat).mag();
    }
  };
}

namespace
{
  static ana::CrossSectionSignal<ana::MuonMomentum>::Registrar MuonMomentumSignal_reg("MuonMomentumSignal");
  static ana::CrossSectionSideband<ana::MuonMomentum>::Registrar MuonMomentumSideband_reg("MuonMomentumSideband");

  static ana::CrossSection2DSignal<ana::MuonPz, ana::MuonPT>::Registrar MuonPzPTSignal_reg("MuonPzPTSignal");
  //TODO: CrossSection2DSideband<>
}

#endif //ANA_MUONMOMENTUM_CPP
