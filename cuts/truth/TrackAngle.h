//File: TrackAngle.h
//Brief: A maximum track angle cut to remove events with muons that are unlikely to make
//       it into MINOS.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_TRACKANGLE_H
#define TRUTH_TRACKANGLE_H

//cut includes
#include "cuts/truth/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace truth
{
  class TrackAngle: public Cut
  {
    public:
      TrackAngle(const YAML::Node& config);
      virtual ~TrackAngle() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const CVUniverse& event) const override;

    private:
      degrees fMax;
  };
}

#endif //TRUTH_TRACKANGLE_H
