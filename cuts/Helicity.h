//File: Helicity.h
//Brief: A Cut on whether this is a neutrino or an anti-neutrino.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_HELICITY_H
#define APO_HELICITY_H

//Cuts includes
#include "Cuts/Cut.h"

namespace apo
{
  class Helicity: public Cut
  {
    public:
      Helicity(const YAML::Node& config);
      virtual ~Helicity() = default;

    protected:
      virtual bool doCut(const SchemaView& event) override;

    private:
      int fHelicity; //Helicity enum from the MINERvA offline framework (Gaudi).
                     //Determines whether this Cut accepts neutrino or anti-neutrino
                     //events.
  };
}

#endif //APO_HELICITY_H
