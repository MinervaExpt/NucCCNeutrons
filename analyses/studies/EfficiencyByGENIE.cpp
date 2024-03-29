//File: EfficiencyByGENIE.cpp
//Brief: Am I less efficient in a particular GENIE category?
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/studies/EfficiencyByGENIE.h"
#include "analyses/studies/TruthInteractionCategories.h"

//util includes
#include "util/Factory.cpp"

namespace ana
{
  EfficiencyByGENIE::EfficiencyByGENIE(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                       std::map<std::string, std::vector<evt::Universe*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fPz(config["Pz"]),
                                                                                                   fPT(config["PT"]),
                                                                                                   fGENIEToEfficiencyNum("Numerator", "Truth #P_z;Truth #P_T",
                                                                                                                         GENIECategories, dir,
                                                                                                                         config["binning"]["Pz"].as<std::vector<double>>(),
                                                                                                                         config["binning"]["PT"].as<std::vector<double>>(),
                                                                                                                         univs),
                                                                                                   fGENIEToEfficiencyDenom("Denominator", "Truth #P_z;Truth #P_T",
                                                                                                                           GENIECategories, dir,
                                                                                                                           config["binning"]["Pz"].as<std::vector<double>>(),
                                                                                                                           config["binning"]["PT"].as<std::vector<double>>(),
                                                                                                                           univs)
  {
  }

  void EfficiencyByGENIE::mcSignal(const evt::Universe& event, const events weight)
  {
    fGENIEToEfficiencyNum[event.GetInteractionType()].Fill(&event, fPz.truth(event), fPT.truth(event), weight);
  }

  void EfficiencyByGENIE::truth(const evt::Universe& event, const events weight)
  {
    fGENIEToEfficiencyDenom[event.GetInteractionType()].Fill(&event, fPz.truth(event), fPT.truth(event), weight);
  }

  void EfficiencyByGENIE::afterAllFiles(const events /*passedSelection*/)
  {
    fGENIEToEfficiencyNum.visit([](auto& hist) { hist.SyncCVHistos(); });
    fGENIEToEfficiencyDenom.visit([](auto& hist) { hist.SyncCVHistos(); });
  }
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::EfficiencyByGENIE> EfficiencyByGENIE_reg("EfficiencyByGENIE");
}
