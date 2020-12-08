//File: Current.h
//Brief: Require that GENIE generated a charged current event.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_CURRENT_H
#define TRUTH_CURRENT_H

//cut includes
#include "cuts/truth/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class Universe;
}

namespace truth
{
  template <int CURRENT>
  class Current: public Cut
  {
    public:
      Current(const YAML::Node& config, const std::string& name): Cut(config, name) {}
      virtual ~Current() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::Universe& event) const override
      {
        return event.GetCurrent() == CURRENT;
      }
  };
}

#endif //TRUTH_CURRENT_H
