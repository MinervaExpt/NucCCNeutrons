//File: Apothem.h
//Brief: Select events within some fiducial apothem. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_APOTHEM_H
#define RECO_APOTHEM_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class Apothem: public Cut
  {
    public:
      Apothem(const YAML::Node& config, const std::string& name);
      virtual ~Apothem() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::CVUniverse& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      const mm fApothem; //Events with primary vertices outside this apothem are cut
      static constexpr double fSlope = -1./sqrt(3.); //A regular hexagon has angles of 2*M_PI/3, so I can find this is 1/tan(M_PI/3.)
  };
}

#endif //RECO_APOTHEM_H
