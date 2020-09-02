//File: Cut.cpp
//Brief: A Cut decides whether a CVUniverse should be considered
//       truth signal.  Cuts can be used to define
//       Signals, Sidebands, and backgrounds.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cuts includes
#include "cuts/truth/Cut.h"

//event includes
#include "evt/CVUniverse.h"

namespace truth
{
  bool Cut::checkConstraint(const evt::CVUniverse& univ) const
  {
    return passesCut(univ);
  }
}
