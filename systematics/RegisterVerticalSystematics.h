//File: RegisterVerticalSystematics.h
//Brief: Any Reweighter<> in the MAT can be used to build a systematic Universe.  This file registers interesting Reweighters-as-systematics.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//NucCCNeutrons includes
#include "evt/Universe.h"

//PlotUtils includes
#include "universes/GenericVerticalSystematic.h"

//c++ includes
#include <memory>

namespace
{
  class MoNANeutronInelasticRegistrar: public plgn::RegistrarBase<evt::Universe, typename evt::Universe::config_t> //PlotUtils::GenericVerticalUniverse<evt::Universe>, typename evt::Universe::config_t>
  {
    public:
      MoNANeutronInelasticRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<evt::Universe, typename evt::Universe::config_t>::instance();
        reg.Add(name, this);
      }

      virtual ~MoNANeutronInelasticRegistrar() = default;

      std::unique_ptr<evt::Universe> NewPlugin(const YAML::Node& config, evt::Universe::config_t chw) override
      {
        return std::unique_ptr<evt::Universe>(new PlotUtils::GenericVerticalUniverse<evt::Universe, PlotUtils::detail::empty>(chw, std::unique_ptr<PlotUtils::Reweighter<evt::Universe, PlotUtils::detail::empty>>(new NeutronInelasticReweighter<evt::Universe>(config.as<std::map<std::string, std::vector<int>>>()))));
      }
  };

  static MoNANeutronInelasticRegistrar regMoNANeutronInelastic("MoNANeutronInelastic");
}
