//File: Sideband.h
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef SIDE_SIDEBAND_H
#define SIDE_SIDEBAND_H

//c++ includes
#include <vector>
#include <memory>

namespace evt
{
  class CVUniverse;
}

namespace bkg
{
  class Background;
}

namespace reco
{
  class Cut;
}

namespace side
{
  class Sideband
  {
    protected:
      using cuts_t = std::vector<std::unique_ptr<reco::Cut>>;
      using backgrounds_t = std::vector<std::unique_ptr<Background>>;

    public:
      Sideband(util::Directory& /*dir*/, cuts_t&& mustPass, const backgrounds_t& /*backgrounds*/, const YAML::Node& /*config*/);
      virtual ~Sideband() = default;

      //In addition to the cuts that an event fails to get into
      //a sideband, a Sideband can also require that events pass
      //specific Cuts.
      cuts_t passes;

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.
      virtual void data(const CVUniverse& event) = 0;
      virtual void truthSignal(const CVUniverse& event) = 0;
      virtual void truthBackground(const CVUnvierse& event, const backgrounds_t::iterator background) = 0;
  };
}

#endif //SIDE_SIDEBAND_H
