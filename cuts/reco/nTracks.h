//File: nTracks.h
//Brief: A number of tracks cut to select events with a given vertex reconstruction quality. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_NTRACKS_H
#define RECO_NTRACKS_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class nTracks: public Cut
  {
    public:
      nTracks(const YAML::Node& config, const std::string& name);
      virtual ~nTracks() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override;

    private:
      long int fMin;
      long int fMax;
  };
}

#endif //RECO_NTRACKS_H
