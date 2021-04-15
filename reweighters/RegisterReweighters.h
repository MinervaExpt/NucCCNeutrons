//File: RegisterReweighters.h
//Brief: Makes Reweighters loadable from YAML files.  I have to tell util::Factory<> about them somewhere, so this is where that happens.  I also translate constructors into YAML::Nodes here with special Registrars.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/Universe.h"

//reweighters includes
#include "reweighters/Reweighter.h"
#include "reweighters/FluxAndCVReweighter.cpp"
#include "reweighters/FSIReweighter.cpp"
#include "reweighters/GeantNeutronCVReweighter.cpp"
#include "reweighters/GENIEReweighter.cpp"
#include "reweighters/LowQ2PiReweighter.cpp"
#include "reweighters/LowRecoil2p2hReweighter.cpp"
#include "reweighters/MKReweighter.cpp"
#include "reweighters/RPAReweighter.cpp"

//util includes
#include "util/Factory.cpp"

namespace
{
  //First, a Registrar for Reweighters that don't need any constructor parameters.
  template <template <class> class REWEIGHT>
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
        return std::unique_ptr<PlotUtils::Reweighter<evt::Universe>>(new REWEIGHT<evt::Universe>());
      }
  };

  static DefaultRegistrar<FluxAndCVReweighter> fluxReg("FluxAndCV");
  static DefaultRegistrar<GeantNeutronCVReweighter> geantReg("GeantNeutronCV");
  static DefaultRegistrar<LowRecoil2p2hReweighter> tune2p2hReg("LowRecoil2p2hReweighter");
  static DefaultRegistrar<MKReweighter> mkReg("MKModel");
  static DefaultRegistrar<RPAReweighter> rpaReg("RPA");

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
}