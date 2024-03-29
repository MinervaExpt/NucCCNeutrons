//File: RegisterReweighters.h
//Brief: Makes Reweighters loadable from YAML files.  I have to tell util::Factory<> about them somewhere, so this is where that happens.  I also translate constructors into YAML::Nodes here with special Registrars.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/Universe.h"

//reweighters includes
#include "PlotUtils/Reweighter.h"
#include "PlotUtils/FluxAndCVReweighter.h"
#include "PlotUtils/FSIReweighter.h"
#include "PlotUtils/GeantNeutronCVReweighter.h"
#include "PlotUtils/GENIEReweighter.h"
#include "PlotUtils/LowQ2PiReweighter.h"
#include "PlotUtils/LowRecoil2p2hReweighter.h"
#include "PlotUtils/MKReweighter.h"
#include "PlotUtils/RPAReweighter.h"
#include "PlotUtils/MINOSEfficiencyReweighter.h"
#include "PlotUtils/SuSAFromValencia2p2hReweighter.h"
#include "weighters/NeutronInelasticReweighter.h"
#include "weighters/BodekRitchieReweighter.h"

//util includes
#include "util/Factory.cpp"

namespace
{
  //First, a Registrar for Reweighters that don't need any constructor parameters.
  template <template <class, class> class REWEIGHT>
  class DefaultRegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      DefaultRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~DefaultRegistrar() = default;

      virtual std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& /*config*/)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new REWEIGHT<evt::Universe, PlotUtils::detail::empty>());
      }
  };

  static DefaultRegistrar<FluxAndCVReweighter> fluxReg("FluxAndCV");
  static DefaultRegistrar<GeantNeutronCVReweighter> geantReg("GeantNeutronCV");
  //static DefaultRegistrar<LowRecoil2p2hReweighter> tune2p2hReg("LowRecoil2p2hWeight");
  static DefaultRegistrar<MKReweighter> mkReg("MKModel");
  static DefaultRegistrar<RPAReweighter> rpaReg("RPA");
  static DefaultRegistrar<MINOSEfficiencyReweighter> minosReg("MINOSEfficiency");
  static DefaultRegistrar<SuSAFromValencia2p2hReweighter> susa2p2hReg("SuSA2p2h");

  //A dedicated Registrar for each Reweighter that needs constructor arguments
  class FSIRegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      FSIRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~FSIRegistrar() = default;

      virtual std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new FSIReweighter<evt::Universe>(config["useElastic"].as<bool>(), config["useAbsorption"].as<bool>()));
      }
  };

  static FSIRegistrar FSIReg("FixFSIBugGENIE");

  class GENIERegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      GENIERegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~GENIERegistrar() = default;

      virtual std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new GENIEReweighter<evt::Universe>(config["useNonResPi"].as<bool>(), config["useDeuteriumPionTune"].as<bool>()));
      }
  };

  static GENIERegistrar regGENIE("GENIEPionTunes");

  class LowQ2PionRegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      LowQ2PionRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~LowQ2PionRegistrar() = default;

      virtual std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new LowQ2PiReweighter<evt::Universe>(config["channel"].as<std::string>()));
      }
  };

  static LowQ2PionRegistrar regLowQ2Pi("LowQ2PionTune");

  class NeutronInelasticRegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      NeutronInelasticRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~NeutronInelasticRegistrar() = default;

      std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new NeutronInelasticReweighter<evt::Universe>(config.as<std::map<std::string, std::vector<int>>>())); //TODO: YAML arguments here
      }
  };

  static NeutronInelasticRegistrar regNeutronInelastic("NeutronInelastic");

  class BodekRitchieRegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      BodekRitchieRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~BodekRitchieRegistrar() = default;

      std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new BodekRitchieReweighter<evt::Universe>(config["mode"].as<int>(1)));
      }
  };

  static BodekRitchieRegistrar regBodekRitchie("BodekRitchieTail");

  class LowRecoil2p2hRegistrar: public plgn::RegistrarBase<PlotUtils::Reweighter<evt::Universe>>
  {
    public:
      LowRecoil2p2hRegistrar(const std::string& name)
      {
        auto& reg = plgn::Factory<PlotUtils::Reweighter<evt::Universe>>::instance();
        reg.Add(name, this);
      }

      virtual ~LowRecoil2p2hRegistrar() = default;

      std::unique_ptr<PlotUtils::Reweighter<evt::Universe>> NewPlugin(const YAML::Node& config)
      {
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new LowRecoil2p2hReweighter<evt::Universe>(config["mode"].as<int>(1)));
      }
  };

  static LowRecoil2p2hRegistrar regLowRecoil2p2hWeight("LowRecoil2p2hWeight");
}
