//File: DeadDiscriminators.h
//Brief: A maximum time difference between a MINOS muon track and a MINERvA muon
//       track to filter out events with bad tracking matches.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_DEADDISCRIMINATORS
#define RECO_DEADDISCRIMINATORS

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class DeadDiscriminators: public Cut
  {
    public:
      DeadDiscriminators(const YAML::Node& config, const std::string& name);
      virtual ~DeadDiscriminators() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::CVUniverse& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      int fMax;
  };
}

#endif //RECO_DEADDISCRIMINATORS
