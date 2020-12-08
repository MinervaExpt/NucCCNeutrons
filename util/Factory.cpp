//File: Factory.cpp
//Brief: A Factory keeps track of all of the plugins of a given type (parameter).  
//       To do this, a Factory must be a singleton, and plugins invoke a macro at 
//       compile time to tell the register about themselves. 
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Inspired by the conversation at https://codereview.stackexchange.com/questions/119812/compile-time-plugin-system
//
//When I create a library of plugins separate from the main() application,
//the linker will often ignore that library and thus not register any
//plugins at runtime.  I've heard of 3 ways to solve this problem so far:
//1) edepViewer's solution: use a symbol from the plugin library in something included in the main() file.  Doesn't seem to work sometimes.
//2) -W1,--whole-archive is supposed to work with gcc a la https://www.bfilipek.com/2018/02/static-vars-static-lib.html#brute-force-code.  Hard to get this to work with CMake
//3) This library makes the plugin library a CMake "OBJECT library".  It doesn't get linked against anything when the library
//target is created.  When I would use the OBJECT library as a dependency in target_link_libraries(), instead go to
//where that target is created and add to its source files $<OBJECT_TARGET:pluginLibraryName>.  Seems like this just
//"combines" the plugin library that would have been generated into another library.  Since this is
//implemented by CMake, I'm hopeful that it works on different platforms.
//Only tested on the minervagpvms = SLF6 so far.

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//c++ includes
#include <memory> //For std::unique_ptr
#include <map>
#ifndef NDEBUG
  #include <iostream>
#endif
#include <utility> //For std::forward<>()

#ifndef PLGN_FACTORY_CPP
#define PLGN_FACTORY_CPP

#ifndef NDEBUG
  #define LOG_DEBUG(msg) std::cerr << msg << "\n";
#else
  #define LOG_DEBUG(msg)
#endif

namespace plgn
{
  template <class BASE, class ...ARGS>
  class RegistrarBase
  {
    public:
      virtual ~RegistrarBase() = default;

      virtual std::unique_ptr<BASE> NewPlugin(const YAML::Node& config, ARGS... args) = 0;
  };

  template <class BASE, class ...ARGS>
  class Factory
  {
    public:
      Factory(Factory&) = delete;
      Factory(Factory&&) = delete;
      Factory& operator =(Factory&) = delete;

      static Factory& instance()
      {
        static Factory reg;
        return reg;
      }

      virtual ~Factory() = default;
 
      void Add(const std::string& name, RegistrarBase<BASE, ARGS...>* reg)
      {
        fNameToReg[name] = reg;
      }

      class NoSuchPlugin: public std::runtime_error
      {
        public:
          NoSuchPlugin(const std::string& name): std::runtime_error(std::string("Couldn't find a plugin of type ") + name), typeName(name) {}
          virtual ~NoSuchPlugin() = default;

          const std::string typeName;
      };

      std::unique_ptr<BASE> Get(const YAML::Node& config, ARGS... args)
      {
        const auto type = config.Tag().substr(1); //yaml-cpp doesn't strip the ! off of tags
        const auto found = fNameToReg.find(type);
        LOG_DEBUG("Setting up a plugin of type " << type)
        try
        {
          if(found != fNameToReg.end()) return found->second->NewPlugin(config, std::forward<ARGS>(args)...);
        }
        catch(const YAML::Exception& e)
        {
          throw std::runtime_error(std::string("Got a YAML error when setting up a ") + type + ": " + e.what());
        }

        throw NoSuchPlugin(type);
      }

    private:
      Factory(): fNameToReg() {}

      std::map<std::string, RegistrarBase<BASE, ARGS...>*> fNameToReg;
  };

  //According to the Stack Overflow article where I learned this technique, interposing 
  //a registrar class between DERIVED and Factory makes sure that expensive plugins don't 
  //have to be instantiated if they are not used.  This is one of the main advantages I want 
  //from my plugin system, and it allows me to create a plugin from a configuration 
  //object when I need it.  
  template <class BASE, class DERIVED, class ...ARGS>
  class Registrar: public RegistrarBase<BASE, ARGS...>
  {
    public:
      Registrar(const std::string& name)
      {
        auto& reg = Factory<BASE, ARGS...>::instance();
        reg.Add(name, this);
        LOG_DEBUG("Registered a plugin named " << name)
      }

      virtual ~Registrar() = default;

      virtual std::unique_ptr<BASE> NewPlugin(const YAML::Node& config, ARGS... args)
      {
        return std::unique_ptr<BASE>(new DERIVED(config, std::forward<ARGS>(args)...));
      }
  };

  //Load a whole YAML map of plugins at once.
  template <class CUT>
  std::vector<std::unique_ptr<CUT>> loadPlugins(const YAML::Node& config)
  {
    std::vector<std::unique_ptr<CUT>> cuts;
    auto& factory = plgn::Factory<CUT>::instance();

    for(auto block: config)
    {
      cuts.emplace_back(factory.Get(block.second));
    }

    return cuts;
  }
}

//Removed because of the complexities of passing in constructor ARGS.  Just instantiate a global static Registrar<> directly.
/*#define REGISTER_PLUGIN(CLASSNAME, BASE) \
namespace \
{ \
  static plgn::Registrar<BASE, CLASSNAME> CLASSNAME_reg(#CLASSNAME); \
}*/

#endif //PLGN_FACTORY_CPP 
