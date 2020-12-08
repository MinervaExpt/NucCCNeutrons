//File: TrackAngle.h
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_TRACKANGLE_H
#define RECO_TRACKANGLE_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class Universe;
}

namespace reco
{
  class TrackAngle: public Cut
  {
    public:
      TrackAngle(const YAML::Node& config, const std::string& name);
      virtual ~TrackAngle() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      degrees fMax;
  };
}

#endif //RECO_TRACKANGLE_H
