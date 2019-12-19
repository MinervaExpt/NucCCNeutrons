//File: AND.cpp
//Brief: A Cut that an event passes if it passes all of its children.  This
//       is a lazy-evaluated AND, so an event will not be passed to any cuts
//       after the first one that it fails.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cuts includes
#include "cuts/AND.h"

//TODO: Directory include

namespace apo
{
  AND::AND(Directory& dir, const YAML::Node& config)
  {
    auto& cutFactory = plgn::Factory<Cut, Directory&>::instance();
    for(const auto& child: config) fRequirements.push_back(std::move(cutFactory.Get(child, dir)));
  }

  bool AND::doCut(const SchemaView& event)
  {
    return std::all_of(fRequirements.begin(), fRequirements.end(), [&event](const auto& req) { return req(event); });
  }
}

//TODO: Declare AND to Factory<Cut, Directory&>
