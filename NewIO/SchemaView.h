//File: SchemaView.h
//Brief: A SchemaView provides a way to access values from the branches in
//       a Schema.  It might cache some of those branches, modify values
//       from another SchemaView, or read directly from a TTree.  It's an
//       abstract base class, but you can also define useful groupings of
//       branches and/or physics calculations here for all SchemaViews to
//       inherit them.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_SCHEMAVIEW_H
#define APO_SCHEMAVIEW_H

//c++ includes
#include <vector>

namespace apo
{
  //A SchemaView is a lightweight view onto the branches of a Schema.
  //It could be implemented by caching, changing, or reading those values
  //from a TTree.
  class SchemaView
  {
    public:
      virtual ~SchemaView() = default;

      virtual double q3() = 0;
      virtual std::vector<int> fsPDGs() = 0;
  };
}

#endif //APO_SCHEMAVIEW_H
