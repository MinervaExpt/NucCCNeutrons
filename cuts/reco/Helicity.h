//File: Helicity.h
//Brief: A Cut on a range of values in reco 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_HELICITY_H
#define RECO_HELICITY_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class Universe;
}

namespace reco
{
  template <int neutrinoType>  //Actually compares to a typedef from MINERvA's Gaudi
                               //framework.
  class Helicity: public Cut
  {
    public:
      Helicity(const YAML::Node& config, const std::string& name): Cut(config, name) {}
      virtual ~Helicity() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override
      {
        return event.GetHelicity() == neutrinoType;
      }
  };
}

#endif //RECO_HELICITY_H
