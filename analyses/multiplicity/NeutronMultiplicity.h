//File: NeutronMultiplicity.cpp
//Brief: A cross section analysis in number of FS neutrons.
//       Neutron candidates are selected by a set of Cuts.
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
  //TODO: Finish defining NeutronMultiplicity.  Looks like I'll need to define a Cut-like base class.
  //A VARIABLE for the CrossSection<> templates
  class NeutronMultiplicity
  {
    public:
      NeutronMultiplicity(const YAML::Node& config): fTruthCuts(plgn::loadPlugins<truth::Cut>(config["truth"])),
                                                     fRecoCuts(plgn::loadPlugins<reco::Cut>(config["reco"]))
      {
      }

      inline std::string name() const { return "Neutron Multiplicity"; }

      neutrons truth(const CVUniverse& event)
      {
        
      }

      neutrons reco(const CVUniverse& event)
      {
      }

    private:
      //TODO: I can't use Cut this way because it takes a CVUniverse instead of a NeutronCandidate.
      //      I need to write my own version of Cut, or make Cut a class template, instead.
      std::vector<std::unique_ptr<truth::Cut>> fTruthCuts;
      std::vector<std::unique_ptr<reco::Cut>> fRecoCuts;
  };

  //Define plugins to implement this analysis
  namespace
  {
    static plgn::Registrar<signal::Signal, sig::CrossSection<ana::NeutronMultiplicity>> NeutronSignal_reg("NeutronMultiplicity");
    static plgn::Registrar<sideband::Sideband, side::CrossSection<ana::NeutronMultiplicity>> NeutronSideband_reg("NeutronMultiplicity");
    static plgn::Registrar<background::Background, bkg::CrossSection<ana::NeutronMultiplicity>> NeutronBackground_reg("NeutronMultiplicity");
  }
}
