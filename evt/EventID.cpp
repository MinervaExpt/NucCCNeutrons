//File: EventID.cpp
//Brief: A unique identifier for a Gaudi PhysicsEvent.  Each entry in an
//       anaTuple is 1 PhysicsEvent, but some PhysicsEvents don't make it to
//       the anaTuple.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/EventID.h"

//c++ includes
#include <iostream>

namespace evt
{
  bool RunID::operator < (const RunID& rhs) const
  {
    return run < rhs.run;
  }

  bool RunID::operator == (const RunID& rhs) const
  {
    return run == rhs.run;
  }

  std::ostream& operator <<(std::ostream& os, const RunID& run)
  {
    return os << "Run " << run.run;
  }

  bool SubrunID::operator < (const SubrunID& rhs) const
  {
    if(!this->RunID::operator==(rhs)) return this->RunID::operator<(rhs);

    return subrun < rhs.subrun;
  }

  bool SubrunID::operator ==(const SubrunID& rhs) const
  {
    return this->RunID::operator==(rhs) && (subrun == rhs.subrun);
  }

  std::ostream& operator <<(std::ostream& os, const SubrunID& subrun)
  {
    return os << static_cast<RunID>(subrun) << " Subrun " << subrun.subrun;
  }

  bool GateID::operator < (const GateID& rhs) const
  {
    if(!this->SubrunID::operator==(rhs)) return this->SubrunID::operator<(rhs);

    return gate < rhs.gate;
  }

  bool GateID::operator ==(const GateID& rhs) const
  {
    return this->SubrunID::operator==(rhs) && (gate == rhs.gate);
  }

  std::ostream& operator <<(std::ostream& os, const GateID& gate)
  {
    return os << static_cast<SubrunID>(gate) << " Gate " << gate.gate;
  }

  bool SliceID::operator < (const SliceID& rhs) const
  {
    if(!this->GateID::operator==(rhs)) return this->GateID::operator<(rhs);
  
    return slice < rhs.slice;
  }

  bool SliceID::operator ==(const SliceID& rhs) const
  {
    return this->GateID::operator==(rhs) && (slice == rhs.slice);
  }

  std::ostream& operator <<(std::ostream& os, const SliceID& slice)
  {
    return os << static_cast<GateID>(slice) << " Slice " << slice.slice;
  }
}
