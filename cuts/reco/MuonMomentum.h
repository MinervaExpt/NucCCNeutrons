//File: MuonMomentum.h
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_MUONMOMENTUM_H
#define RECO_MUONMOMENTUM_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class MuonMomentum: public Cut
  {
    public:
      MuonMomentum(const YAML::Node& config, const std::string& name);
      virtual ~MuonMomentum() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override;

    private:
      GeV fMin;
      GeV fMax;
  };
}

#endif //RECO_MUONMOMENTUM_H
