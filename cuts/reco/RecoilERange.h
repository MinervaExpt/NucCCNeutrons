//File: RecoilERange.h
//Brief: A Cut on a range of values in reco recoil energy.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_RECOILERANGE_H
#define RECO_RECOILERANGE_H

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  class RecoilERange: public Cut
  {
    public:
      RecoilERange(const YAML::Node& config, const std::string& name);
      virtual ~RecoilERange() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override;

    private:
      GeV fMin;
      GeV fMax;
  };
}

#endif //RECO_RECOILERANGE_H
