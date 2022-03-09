//File: FailsQENeutronKinematics.h
//Brief: Remove events with neutron candidates that are likely to be from a neutral pion. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_FAILSQENEUTRONKINEMATICS
#define RECO_FAILSQENEUTRONKINEMATICS

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//cut includes
#include "cuts/reco/Cut.h"

namespace evt
{
  class Universe;
}

namespace reco
{
  class FailsQENeutronKinematics: public Cut
  {
    public:
      FailsQENeutronKinematics(const YAML::Node& config, const std::string& name);
      virtual ~FailsQENeutronKinematics() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      ana::NeutronMultiplicity fCandSelector;

      double fBlobCosineDiffMin; //Minimum blob angle where neutron-rich candidates begin

      MeV calcENuQE(const evt::Universe& univ) const;
  };
}

#endif //RECO_FAILSQENEUTRONKINEMATICS
