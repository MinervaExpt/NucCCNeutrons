//File: Background.h
//Brief: A plugin for making plots for events that end up in a background category.
//       Backgrounds are defined by a set of cuts that an event must pass.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef BKG_BACKGROUND_H
#define BKG_BACKGROUND_H

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//c++ includes
#include <vector>
#include <memory>

namespace util
{
  class Directory;
}

namespace evt
{
  class CVUniverse;
}

namespace truth
{
  class Cut;
}

namespace bkg
{
  class Background
  {
    public:
      using cuts_t = std::vector<std::unique_ptr<truth::Cut>>;

      Background(const YAML::Node& /*config*/, util::Directory& /*dir*/, const std::string& name, cuts_t&& mustPass, std::vector<evt::CVUniverse*>& universes);
      virtual ~Background() = default;

      //Cuts an event must pass to be classified as this background
      cuts_t passes;

      //The event loop will call this interface for MC events that qualify for this Background.
      //Implement it to Fill() your background histograms.
      virtual void Fill(const evt::CVUniverse& event) = 0;

      //Get the name of this Background
      const std::string& name() { return fName; }

    private:
      const std::string fName;
  };
}

#endif //BKG_BACKGROUND_H
