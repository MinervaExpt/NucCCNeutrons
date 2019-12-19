//File: Schema.cpp
//Brief: A Schema lists all of the branches that this analysis and any related
//       studies might ever read.  It provides a place for those branches to be
//       mapped to memory locations instead of names that have to be looked up
//       by a map<>.  If you add branches here, you also need to add things to
//       SchemaView, NotCached, and Universe to use them.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//local includes
#include "NewIO/Schema.h"

//ROOT includes
#include "TTree.h"

namespace apo
{
  Schema::Schema(const TTree& tree) //TODO: Set up BranchPtr<>s
  {
  }
}
