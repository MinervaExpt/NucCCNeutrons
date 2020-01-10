//File: MinosDeltaT.h
//Brief: A maximum time difference between a MINOS muon track and a MINERvA muon
//       track to filter out events with bad tracking matches.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_MINOSDELTAT_H
#define RECO_MINOSDELTAT_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class MinosDeltaT: public Cut
  {
    public:
      MinosDeltaT(const YAML::Node& config);
      virtual ~MinosDeltaT() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const CVUniverse& event) const override;

    private:
      ns fMax;
  };
}

#endif //RECO_MINOSDELTAT_H
