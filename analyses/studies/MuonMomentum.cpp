//File: MuonMomentum.cpp
//Brief: A MuonMomentum VARIABLE to demonstrate my analysis machinery.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/studies/CrossSectionSignal.h"
#include "analyses/studies/CrossSectionSideband.h"

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
}

namespace
{
  static plgn::Registrar<ana::Study, ana::CrossSectionSignal<ana::MuonMomentum>, util::Directory&,
                         typename ana::Study::cuts_t&&, std::vector<typename ana::Study::background_t>&,
                         std::map<std::string, std::vector<evt::CVUniverse*>>&> MuonMomentumSignal_reg("MuonMomentumSignal");
  static plgn::Registrar<ana::Study, ana::CrossSectionSideband<ana::MuonMomentum>, util::Directory&,
                         typename ana::Study::cuts_t&&, std::vector<typename ana::Study::background_t>&,
                         std::map<std::string, std::vector<evt::CVUniverse*>>&> MuonMomentumSideband_reg("MuonMomentumSideband");
}

#endif //ANA_MUONMOMENTUM_CPP
