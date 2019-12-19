//File: AND.h
//Brief: A Cut that an event passes if it passes all of its children.  This
//       is a lazy-evaluated AND, so an event will not be passed to any cuts
//       after the first one that it fails.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_AND_H
#define APO_AND_H

//cuts includes
#include "cuts/Cut.h"

namespace apo
{
  class AND: public Cut
  {
    public:
      AND(Directory& dir, const YAML::Node& config);
      virtual ~AND() = default;
  
    protected:
      virtual bool doCut(const SchemaView& event) override;
  
    private:
      std::vector<std::unique_ptr<Cut>> fRequirements; //An event must pass all of
                                                       //fRequirements to pass this Cut
  };
}

#endif //APO_AND_H
