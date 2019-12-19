//File: Cut.cpp
//Brief: A Cut decides whether a NucCCNeutron event, accessed through a SchemaView,
//       should be passed on to a Study or not.  Each Cut will also keep track of
//       how many times events pass and fail it to produce a cut summary table.
//       Derive from this abstract base class to use it.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//TODO: Do I need any includes for YAML::Node?

//NewIO includes
#include "NewIO/SchemaView.h"

namespace apo
{
  bool Cut::operator ()(const SchemaView& event)
  {
    //TODO: Record statistics on passed and failed events here
    return doCut(event);
  }
}
