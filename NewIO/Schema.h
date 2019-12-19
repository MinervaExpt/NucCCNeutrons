//File: Schema.h
//Brief: A Schema lists all of the branches that this analysis and any related
//       studies might ever read.  It provides a place for those branches to be
//       mapped to memory locations instead of names that have to be looked up
//       by a map<>.  If you add branches here, you also need to add things to
//       SchemaView, NotCached, and Universe to use them.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_SCHEMA_H
#define APO_SCHEMA_H

//util includes
#include "utils/BranchPtr.h"

//Forward declarations for ROOT
class TTree;

namespace apo
{
  //A Schema maps branches that need to be looked up to locations in memory that are fixed at compile-time.
  //I plan to create one Schema each time a new TTree is opened.
  //Nota Bene: You could even implement Schema itself with ChainWrapper!
  class Schema
  {
    public:
      Schema(const TTree& tree); //Set up the BranchPtr<>s.
  
      //Every branch this analysis will ever read
      BranchPtr<double> q3{"NucCCNeutrons_q3"};
      BranchPtr<std::vector<int>> fsPDGs{"truth_FS_PDG_codes"};
  };
}

#endif //APO_SCHEMA_H
