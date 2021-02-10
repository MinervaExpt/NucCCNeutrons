//File: NoPi0s.h
//Brief: Select events with no neutral pions in the final state. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_NOPI0S_H
#define TRUTH_NOPI0S_H

//cut includes
#include "cuts/truth/Cut.h"

namespace evt
{
  class Universe;
}

namespace truth
{
  class NoPi0s: public Cut
  {
    public:
      NoPi0s(const YAML::Node& config, const std::string& name);
      virtual ~NoPi0s() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::Universe& event) const override;

    private:
      struct FSPart
      {
        int pdgCode;
        GeV energy;
      };
  };
}

#endif //TRUTH_NOPI0S_H
