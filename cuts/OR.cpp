//File: OR.cpp
//Brief: A Cut that an event passes if it passes any of its children.  This
//       is a lazy-evaluated OR, so an event will be assigned to the first
//       child Cut that it passes.
//
//       OR Cuts model the categories of other analyses.  The child Cuts of
//       an OR will each be in a subdirectory of the Directory passed to this
//       OR Cut.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cuts includes
#include "cuts/OR.h"

//TODO: include something for Directory

namespace apo
{
  OR::OR(Directory& dir, const YAML::Node& config)
  {
    auto& cutFactory = plgn::Factory<Cut, Directory&>::instance();
    for(const auto& child: config) fAlternatives.push_back(std::move(cutFactory.Get(child.first, dir.mkdir(child.first))));
  }

  bool doCut(const SchemaView& event)
  {
    return std::any_of(fAlternatives.begin(), fAlternatives.end(), [&event](const auto& alt){ return alt(event); });
  }
}

//TODO: Declare this class to Factory<Cut, Directory&>
