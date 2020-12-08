//File: MuonZMomentum.h
//Brief: A minimum muon momentum cut to get rid of events that MINOS doesn't reconstruct very well.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_MUONZMOMENTUM_H
#define TRUTH_MUONZMOMENTUM_H

//cut includes
#include "cuts/truth/Cut.h"

namespace truth
{
  class MuonZMomentum: public Cut
  {
    public:
      MuonZMomentum(const YAML::Node& config, const std::string& name);
      virtual ~MuonZMomentum() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::Universe& event) const override;

    private:
      GeV fMin;
  };
}

#endif //TRUTH_MUONZMOMENTUM_H
