//File: MuonMomentum.cpp
//Brief: A MuonMomentum VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/signal/CrossSection.h"
#include "analyses/sideband/CrossSection.h"
#include "analyses/background/CrossSection.h"

//evt includes
#include "evt/CVUniverse.h"

//cuts includes
#include "cuts/reco/Cut.h"

namespace ana
{
  //A muon momentum magnitude VARIABLE for the CrossSection<> templates.
  struct MuonMomentum
  {
    MuonMomentum(const YAML::Node& /*config*/) = default;

    inline std::string name() const { return "Muon Momentum"; }

    GeV truth(const CVUniverse& event)
    {
      return event.GetTruthPmu();
    }

    GeV reco(const CVUniverse& event)
    {
      return MeV(event.GetPmu()).in<GeV>(); //Put the NS Framework value into the units that truth uses
    }
  };

  namespace
  {
    static plgn::Registrar<signal::Signal, sig::CrossSection<ana::MuonMomentum>> MuonMomentumSignal_reg("MuonMomentum");
    static plgn::Registrar<sideband::Sideband, side::CrossSection<ana::MuonMomentum>> MuonMomentumSideband_reg("MuonMomentum");
    static plgn::Registrar<background::Background, bkg::CrossSection<ana::MuonMomentum>> MuonMomentumBackground_reg("MuonMomentum");
  }
}
