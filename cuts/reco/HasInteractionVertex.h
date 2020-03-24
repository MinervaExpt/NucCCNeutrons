//File: HasInteractionVertex.h
//Brief: Require that a CCQENu event has a reconstructed muon. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_HASINTERACTIONVERTEX_H
#define RECO_HASINTERACTIONVERTEX_H

//cut includes
#include "cuts/reco/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace reco
{
  class HasInteractionVertex: public Cut
  {
    public:
      HasInteractionVertex(const YAML::Node& config, const std::string& name);
      virtual ~HasInteractionVertex() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override;
  };
}

#endif //RECO_HASINTERACTIONVERTEX_H
