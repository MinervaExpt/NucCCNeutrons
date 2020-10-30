//File: Helicity.h
//Brief: A Cut on a range of values in truth 3-momentum transfer.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef TRUTH_HELICITY_H
#define TRUTH_HELICITY_H

//cut includes
#include "cuts/truth/Cut.h"

//util includes
#include "util/Factory.cpp"

//TODO: Do I need any includes for YAML::Node?
namespace evt
{
  class CVUniverse;
}

namespace truth
{
  template <int neutrinoPDG>
  class Helicity: public Cut
  {
    public:
      Helicity(const YAML::Node& config, const std::string& name): Cut(config, name) {}
      virtual ~Helicity() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool passesCut(const evt::CVUniverse& event) const override
      {
        return event.GetTruthNuPDG() == neutrinoPDG;
      }
  };
}

#endif //TRUTH_HELICITY_H
