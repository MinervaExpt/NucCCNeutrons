//File: OR.h
//Brief: A Cut that an event passes if it passes any of its children.  This
//       is a lazy-evaluated OR, so an event will be assigned to the first
//       child Cut that it passes.
//
//       OR Cuts model the categories of other analyses.  The child Cuts of
//       an OR will each be in a subdirectory of the Directory passed to this
//       OR Cut.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_OR_H
#define APO_OR_H

//cuts includes
#include "cuts/Cut.h"

namespace apo
{
  class OR: public Cut
  {
    public:
      OR(Directory& dir, const YAML::Node& config);
      virtual ~OR() = default;
  
    protected:
      virtual bool doCut(const SchemaView& event) override;
  
    private:
      std::vector<std::unique_ptr<Cut>> fAlternatives; //The alternative Cuts that could
                                                       //result in an event passing this Cut.
  };
}

#endif //APO_OR_H
