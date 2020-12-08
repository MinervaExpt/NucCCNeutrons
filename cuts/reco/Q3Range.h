//File: Q3Range.h
//Brief: A Cut on a range of values in reco 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_Q3RANGE_H
#define RECO_Q3RANGE_H

//analyses includes
#include "analyses/studies/q3.cpp"

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  //TODO: This could easily be a class template on Universe
  //      for other analyses to use it.
  class Q3Range: public Cut
  {
    public:
      Q3Range(const YAML::Node& config, const std::string& name);
      virtual ~Q3Range() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      ana::q3 fCalc;
      GeV fMin;
      GeV fMax;
  };
}

#endif //RECO_Q3RANGE_H
