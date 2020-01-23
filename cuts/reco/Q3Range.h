//File: Q3Range.h
//Brief: A Cut on a range of values in reco 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_Q3RANGE_H
#define RECO_Q3RANGE_H

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  //TODO: This could easily be a class template on CVUniverse
  //      for other analyses to use it.
  class Q3Range: public Cut
  {
    public:
      Q3Range(const YAML::Node& config, const std::string& name);
      virtual ~Q3Range() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override;

    private:
      GeV fMin;
      GeV fMax;
  };
}

#endif //RECO_Q3RANGE_H
