//File: MuonMomentum.h
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_MUONMOMENTUM_H
#define TRUTH_MUONMOMENTUM_H

//cut includes
#include "cuts/truth/Cut.h"

namespace truth
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

#endif //TRUTH_MUONMOMENTUM_H
