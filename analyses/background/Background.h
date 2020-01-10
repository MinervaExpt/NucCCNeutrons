//File: Background.h
//Brief: A plugin for making plots for events that end up in a background category.
//       Backgrounds are defined by a set of cuts that an event must pass.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef BKG_BACKGROUND_H
#define BKG_BACKGROUND_H

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
  class Background
  {
    private:
      using cuts_t = std::vector<std::unique_ptr<truth::Cut>>;

    public:
      Background(Directory& /*dir*/, cuts_t&& mustPass, const YAML::Node& /*config*/);
      virtual ~Background() = default;

      //Cuts an event must pass to be classified as this background
      cuts_t passes;

      //The event loop will call this interface for MC events that qualify for this Background.
      //Implement it to Fill() your background histograms.
      virtual void Fill(const evt::CVUniverse& event) = 0;
  };
}

#endif //BKG_BACKGROUND_H
