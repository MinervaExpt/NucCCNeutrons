//File: Sideband.h
//Brief: A place to Fill() histograms with events that pass all signal cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef SIDE_SIDEBAND_H
#define SIDE_SIDEBAND_H

//c++ includes
#include <vector>
#include <map>
#include <memory>

namespace YAML
{
  class Node;
}

namespace util
{
  class Directory;
}

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
    public:
      using cuts_t = std::vector<std::unique_ptr<reco::Cut>>;
      using background_t = std::unique_ptr<bkg::Background>;

      Sideband(const YAML::Node& /*config*/, util::Directory& /*dir*/, cuts_t&& mustPass,
               const std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::CVUniverse*>>& universes);
      virtual ~Sideband() = default;

      //In addition to the cuts that an event fails to get into
      //a sideband, a Sideband can also require that events pass
      //specific Cuts.
      cuts_t passes;

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.  Each universe in univs is
      //guaranteed by the event loop to givethe same physics variables
      //EXCEPT WEIGHT.  So, calculate your physics variables only
      //once and then loop over univs to Fill() histograms.
      virtual void data(const std::vector<evt::CVUniverse*>& univs) = 0;
      virtual void truthSignal(const std::vector<evt::CVUniverse*>& univs) = 0;
      virtual void truthBackground(const std::vector<evt::CVUniverse*>& univs, const background_t& background) = 0;
  };
}

#endif //SIDE_SIDEBAND_H
