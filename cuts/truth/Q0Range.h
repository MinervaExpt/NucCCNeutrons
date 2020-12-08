//File: Q0Range.h
//Brief: A Cut on a range of values in truth available energy.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_Q0RANGE_H
#define TRUTH_Q0RANGE_H

//cut includes
#include "cuts/truth/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class Universe;
}

namespace truth
{
  //TODO: This could easily be a class template on Universe
  //      for other analyses to use it.
  class Q0Range: public Cut
  {
    public:
      Q0Range(const YAML::Node& config, const std::string& name);
      virtual ~Q0Range() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::Universe& event) const override;

    private:
      GeV fMin;
      GeV fMax;
  };
}

#endif //TRUTH_Q0RANGE_H
