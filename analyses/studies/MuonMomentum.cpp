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
#include "evt/CVUniverse.h"

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

    GeV truth(const evt::CVUniverse& event)
    {
      return event.GetTruthPmu().p().mag();
    }

    GeV reco(const evt::CVUniverse& event)
    {
      return event.GetMuonP().p().mag();
    }
  };

  //A muon p_z VARIABLE
  struct MuonPz
  {
    MuonPz(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "Muon p_z"; }

    GeV truth(const evt::CVUniverse& event)
    {
      return event.GetTruthPmu().p().z();
    }

    GeV reco(const evt::CVUniverse& event)
    {
      return event.GetMuonP().p().z();
    }
  };

  //A muon p_transverse VARIABLE
  struct MuonPT
  {
    MuonPT(const YAML::Node& /*config*/) {}

    inline std::string name() const { return "Muon p_T"; }

    GeV truth(const evt::CVUniverse& event)
    {
      const units::XYZVector<GeV> pz = {0_GeV, 0_GeV, event.GetTruthPmu().p().z()};
      return event.GetTruthPmu().p().cross(pz.unit()).mag(); //TODO: does unit() return the wrong units?
    }
    
    GeV reco(const evt::CVUniverse& event)
    {
      const units::XYZVector<GeV> pz = {0_GeV, 0_GeV, event.GetMuonP().p().z()};
      return event.GetMuonP().p().cross(pz.unit()).mag();
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
