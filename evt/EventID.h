//File: EventID.h
//Brief: A unique identifier for a Gaudi PhysicsEvent.  Each entry in an
//       anaTuple is 1 PhysicsEvent, but some PhysicsEvents don't make it to
//       the anaTuple.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef EVT_EVENTID_H
#define EVT_EVENTID_H

//c++ includes
#include <functional>

namespace evt
{
  struct RunID
  {
    int run;

    bool operator < (const RunID& rhs) const;
    bool operator == (const RunID& rhs) const;
  };

  std::ostream& operator <<(std::ostream& os, const RunID& run);

  struct SubrunID: public RunID
  {
    int subrun;

    bool operator < (const SubrunID& rhs) const;
    bool operator == (const SubrunID& rhs) const;
  };

  std::ostream& operator <<(std::ostream& os, const SubrunID& run);

  struct GateID: public SubrunID
  {
    int gate;

    bool operator < (const GateID& rhs) const;
    bool operator == (const GateID& rhs) const;
  };

  std::ostream& operator <<(std::ostream& os, const GateID& run);

  struct SliceID: public GateID
  {
    int slice;

    bool operator < (const SliceID& rhs) const;
    bool operator == (const SliceID& rhs) const;
  };

  std::ostream& operator <<(std::ostream& os, const SliceID& run);
}

namespace std
{
  //We need a std::hash<> specialization for this to work with std::unordered_map<>
  template <>
  struct hash<evt::RunID>
  {
    size_t operator ()(const evt::RunID& val) const { return val.run; }
  };

  template <>
  struct hash<evt::SubrunID>
  {
    size_t operator ()(const evt::SubrunID& val) const { return val.run*1e6 + val.subrun; }
  };

  template <>
  struct hash<evt::GateID>
  {
    size_t operator ()(const evt::GateID& val) const { return val.run*1e10 + val.subrun*1e4 + val.gate; }
  };

  template <>
  struct hash<evt::SliceID>
  {
    size_t operator ()(const evt::SliceID& val) const { return val.run*1e12 + val.subrun*1e6 + val.gate*1e2 + val.slice; }
  };
}

#endif //EVT_EVENTID_H
