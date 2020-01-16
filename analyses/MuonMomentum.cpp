//File: MuonMomentum.cpp
//Brief: A MuonMomentum VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <string>

//analyses includes
//That's right, I'm covering up a base class function.  Suppress the warnings from it because "I know what I'm doing".
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "analyses/signal/CrossSection.h"
#include "analyses/sideband/CrossSection.h"
#include "analyses/background/CrossSection.h"
#pragma GCC diagnostic pop

//evt includes
#include "evt/CVUniverse.h"

//cuts includes
#include "cuts/reco/Cut.h"
#include "cuts/truth/Cut.h"

//util includes
#include "util/Factory.cpp"

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
      //TODO: Is GetPmu() shifted appropriately?
      return MeV(event.GetPmu()).in<GeV>(); //Put the NS Framework value into the units that truth uses
    }
  };

  namespace
  {
    static plgn::Registrar<sig::Signal, sig::CrossSection<ana::MuonMomentum>, util::Directory&,
                           std::vector<evt::CVUniverse*>&> MuonMomentumSignal_reg("MuonMomentum");
    static plgn::Registrar<side::Sideband, side::CrossSection<ana::MuonMomentum>, util::Directory&,
                           typename side::Sideband::cuts_t&&, std::vector<typename side::Sideband::background_t>&,
                           std::vector<evt::CVUniverse*>&> MuonMomentumSideband_reg("MuonMomentum");
    static plgn::Registrar<bkg::Background, bkg::CrossSection<ana::MuonMomentum>, util::Directory&,
                           std::string&, typename bkg::Background::cuts_t&&, std::vector<evt::CVUniverse*>&> MuonMomentumBackground_reg("MuonMomentum");
  }
}
