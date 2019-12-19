//File: NotCached.cpp
//Brief: A NotCached is a SchemaView that reads directly from a TTree.
//       So, if you use a NotCached repeatedly for multiple systematic
//       universes, you're doing whatever ROOT does to read a value
//       from a TTree once for each universe for each value you use.
//       Study-specific caches should derive from NotCached and cache
//       the values those Studies use.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//NewIO includes
#include "NewIO/NotCached.h"
#include "NewIO/Schema.h"

namespace apo
{
  NotCached::NotCached(const Schema& schema): fSchema(schema)
  {
  }
}
