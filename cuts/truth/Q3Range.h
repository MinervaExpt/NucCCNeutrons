//File: Q3Range.h
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_Q3RANGE_H
#define TRUTH_Q3RANGE_H

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
  class Q3Range: public Cut
  {
    public:
      Q3Range(const YAML::Node& config);
      virtual ~Q3Range() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const CVUniverse& event) const override;

    private:
      GeV fMin;
      GeV fMax;
  };
}

#endif //TRUTH_Q3RANGE_H
