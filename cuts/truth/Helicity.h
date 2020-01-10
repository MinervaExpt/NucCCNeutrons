//File: Helicity.h
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_HELICITY_H
#define TRUTH_HELICITY_H

//cut includes
#include "cuts/truth/Cut.h"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace truth
{
  template <int neutrinoType>  //Actually compares to a typedef from MINERvA's Gaudi
                               //framework.
  class Helicity: public Cut
  {
    public:
      Helicity(const YAML::Node& config);
      virtual ~Helicity() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const CVUniverse& event) const override
      {
        return event.GetTruthHelicity() == neutrinoType;
      }
  };
}

#endif //TRUTH_HELICITY_H
