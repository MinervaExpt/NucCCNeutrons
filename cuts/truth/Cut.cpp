//File: Cut.cpp
//Brief: A Cut decides whether a CVUniverse should be considered
//       truth signal.  Cuts can be used to define
//       Signals, Sidebands, and backgrounds.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//event includes
#include "evt/CVUniverse.h"

namespace truth
{
  bool Cut::operator ()(const CVUniverse& event)
  {
    //TODO: Record statistics on passed and failed events here
    return passesCut(event);
  }
}
