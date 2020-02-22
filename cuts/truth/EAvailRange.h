//File: EAvailRange.h
//Brief: A Cut on a range of values in truth available energy.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_EAVAILRANGE_H
#define TRUTH_EAVAILRANGE_H

//cut includes
#include "cuts/truth/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace truth
{
  //TODO: This could easily be a class template on CVUniverse
  //      for other analyses to use it.
  class EAvailRange: public Cut
  {
    public:
      EAvailRange(const YAML::Node& config);
      virtual ~EAvailRange() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override;

    private:
      GeV fMin;
      GeV fMax;
  };
}

#endif //TRUTH_EAVAILRANGE_H
