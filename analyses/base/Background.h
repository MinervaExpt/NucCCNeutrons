//File: Background.h
//Brief: A set of cuts with a name that defines a background category.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef BKG_BACKGROUND_H
#define BKG_BACKGROUND_H

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//c++ includes
#include <vector>
#include <memory>

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
  struct Background
  {
    public:
      using cuts_t = std::vector<std::unique_ptr<truth::Cut>>;

      //Configuration from a YAML::Node
      Background(const std::string& name, const YAML::Node& config);

      const std::string& name() const { return fName; }

      //Cuts an event must pass to be classified as this background
      cuts_t passes;

    private:
      const std::string fName;
  };
}

#endif //BKG_BACKGROUND_H
