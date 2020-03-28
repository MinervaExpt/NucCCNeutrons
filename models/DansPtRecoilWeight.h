//File: DansPtRecoilWeight.h
//Brief: A DansPtRecoilWeight model modifies the CV's weight to account for a better-modeled or better-constrained
//       flux.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef MODEL_DANSPTRECOILWEIGHT
#define MODEL_DANSPTRECOILWEIGHT

//models includes
#include "models/Model.h"

namespace model
{
  class DansPtRecoilWeight: public Model
  {
    public:
      DansPtRecoilWeight(const YAML::Node& config);
      virtual ~DansPtRecoilWeight() = default;

      virtual events GetWeight(const evt::CVUniverse& univ) const override;

    private:
      bool hasTrueSingleChargedPion(const evt::CVUniverse& univ) const;
      bool hasTrueSingleNeutralPion(const evt::CVUniverse& univ) const;
  };
}

#endif //MODEL_DANSPTRECOILWEIGHT
