//File: NotCached.h
//Brief: A NotCached is a SchemaView that reads directly from a TTree.
//       So, if you use a NotCached repeatedly for multiple systematic
//       universes, you're doing whatever ROOT does to read a value
//       from a TTree once for each universe for each value you use.
//       Study-specific caches should derive from NotCached and cache
//       the values those Studies use.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_NOTCACHED_H
#define APO_NOTCACHED_H

//NewIO includes
#include "NewIO/SchemaView.h"

namespace apo
{
  //Worst-case Schema access: read the branches every time
  class NotCached: public SchemaView
  {
    public:
      NotCached(const Schema& schema);
      virtual ~NotCached() = default;
  
      virtual double q3() override { return fSchema.q3(); }
      virtual std::vector<int> fsPDGs() override { return fSchema.fsPDGs(); }
  
    private:
      Schema& fSchema; //Branches from which all values will come
  };
}

#endif //APO_NOTCACHED_H
