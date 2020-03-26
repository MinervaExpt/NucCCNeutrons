//File: EfficiencyByGENIE.cpp
//Brief: Am I less efficient in a particular GENIE category?
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/studies/EfficiencyByGENIE.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  //GENIE interaction types for GENIE 2.8.12 according to Dan.  I'm matching an enum that the Gaudi
  //framework gets from GENIE.
  std::map<int, std::string> GENIECategories = {{1, "QE"},
                                                {8, "2p2h"},
                                                {2, "RES"},
                                                {3, "DIS"}
                                                //Other is built in for free
                                               };
}

namespace ana
{
  EfficiencyByGENIE::EfficiencyByGENIE(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, std::vector<background_t>& backgrounds,
                                       std::map<std::string, std::vector<evt::CVUniverse*>>& univs): Study(config, dir, std::move(mustPass), backgrounds, univs),
                                                                                                   fPz(config["Pz"]),
                                                                                                   fPT(config["PT"]),
                                                                                                   fGENIEToEfficiencyNum(GENIECategories, dir, "Numerator",
                                                                                                                         "Truth #P_z;Truth #P_T",
                                                                                                                         config["binning"]["Pz"].as<std::vector<double>>(),
                                                                                                                         config["binning"]["PT"].as<std::vector<double>>(),
                                                                                                                         univs),
                                                                                                   fGENIEToEfficiencyDenom(GENIECategories, dir, "Denominator",
                                                                                                                           "Truth #P_z;Truth #P_z",
                                                                                                                           config["binning"]["Pz"].as<std::vector<double>>(),
                                                                                                                           config["binning"]["PT"].as<std::vector<double>>(),
                                                                                                                           univs)
  {
  }

  void EfficiencyByGENIE::mcSignal(const evt::CVUniverse& event, const events weight)
  {
    fGENIEToEfficiencyNum[event.GetInteractionType()].Fill(&event, fPz.truth(event), fPT.truth(event), weight);
  }

  void EfficiencyByGENIE::truth(const evt::CVUniverse& event, const events weight)
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
