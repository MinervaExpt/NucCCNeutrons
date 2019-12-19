//File: Cut.h
//Brief: A Cut decides whether a NucCCNeutron event, accessed through a SchemaView,
//       should be passed on to a Study or not.  Each Cut will also keep track of
//       how many times events pass and fail it to produce a cut summary table.
//       Derive from this abstract base class to use it.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_CUT_H
#define APO_CUT_H

//TODO: Do I need any includes for YAML::Node?

namespace apo
{
  class SchemaView;

  class Cut
  {
    public:
      Cut(const YAML::Node& /*config*/) = default;
      virtual ~Cut() = default;

      //TODO: Return a Study from operator () that happens to be
      //      nullptr if failed this Cut?  Things I've thought of
      //      so far:
      //Not every Cut can return a Study directly:
      //- an OR NEVER owns a Study
      //- an AND NEVER owns a Study
      //- only the last child of an AND can own a Study
      //- each child of an OR can own a Study
      //
      //Thus:
      //- not every Cut owns a Study
      //- ANDs and ORs are essential to understanding which Cuts can
      //  own a Study
      //- A Cut owns a Study if it has no children and is not part of
      //  an AND
      //
      //Remember that I want an AND to be able to "branch" if any of
      //its nodes, not necessarily even the last node, is an OR.  So,
      //an AND can even have the potential to return one of several Studies.
      //
      //So, an AND might have multiple Directories to pass from one child
      //to the next.  Maybe this is indicating that it doesn't make sense
      //for an AND to have an OR as a child?  This seems like a powerful
      //feature if I can make it work though.  Maybe every Cut can generate
      //Directories as part of an extended setup procedure?
      //
      //Another rule I thought of: an AND never needs to have another AND
      //as a direct child.
      //
      //A Cut is always a direct child of either an AND or an OR.  However,
      //an AND has to figure out which branch of an OR to pass back a Study
      //from.  Seems like I could get somewhere by implementing OR first.
      //
      //OR could be implemented by returning the Study from the child that
      //the event first passed.  AND has to be more complicated if I'm going
      //to allow it to have ORs non-terminal as children.  OR will not work
      //by simply storing a Study for each child because children could themselves
      //be ORs.
      //
      //An AND isn't really a logical multi-channel AND at all.  It needs to
      //implement a tree of cuts.  Perhaps each Cut needs to hold a pointer
      //to another Cut.  Maybe I could implement this structure with:
      //- every Cut's operator () returns a Study
      //- every Cut owns the next Cut(s).  AND and OR
      //  just construct Cuts appropriately.
      //- every Cut with no children has an Other as
      //  a child.  Other is the only Cut that owns a Study.
      //- This sounds like "every Cut is a node", so maybe I can still
      //  separate some features out of Cut itself.
      //- Both of these features are powerful and useful: defining alternatives
      //  in c++ and defining alternatives in YAML.  The latter seems to require
      //  overriding operator () itself rather than just operator ().  The second
      //  feature can just be implemented in a Study itself though.
      //
      //I could implement this "Other as default child" behavior with a default
      //constructor for Cut.
      //
      //TODO: There's a bug in these steps when an AND interacts with an OR.
      //Steps to construct a tree:
      //- An AND is a "head" node.  It creates its first child and passes it the list of
      //  remaining AND children.
      //TODO: This list needs to be emptied, but it shouldn't end up as the children of
      //      an OR.
      //- A Cut has a list of children.  The Cut default constructor creates the first
      //  child in that list of children.  If the list is empty, it creates an Other and
      //  effectively owns a Study instead.
      //- An OR creates one Cut for each of its children.  Each Cut gets its own configuration
      //  map as its list of children.
      //TODO: What happens when an OR is the child of an AND?
      //- operator () returns a child's operator().  Other::operator ()() returns that Other's
      //  own Study.
      bool operator ()(const SchemaView& event);

    protected:
      //Your concrete Cut class must override this method.
      virtual bool doCut(const SchemaView& event) = 0;
  };
}

#endif //APO_CUT_H
