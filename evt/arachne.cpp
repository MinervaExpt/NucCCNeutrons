//File: arachne.cpp
//Brief: A function to generate links to MINERvA's online event display.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/arachne.h"
#include "evt/EventID.h"

namespace util
{
  std::string arachne(const evt::SliceID& id, const bool isData, const bool /*useRodriges*/)
  {
    return std::string("http://minerva05.fnal.gov/Arachne/arachne.html?det=") + (isData?"MV":"SIM_minerva") +
                       "&recoVer=v21r1p1&run=" + std::to_string(id.run) +
                       "&subrun=" + std::to_string(id.subrun) +
                       "&gate=" + std::to_string(id.gate + !isData) +
                       "&slice=" + std::to_string(id.slice);
  }
}
