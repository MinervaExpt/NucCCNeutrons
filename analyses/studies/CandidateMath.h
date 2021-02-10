//File: CandidateMath.h
//Brief: Useful functions for calculating neutron candidate observables.  InvariantMass() assumes
//       that both neutron candidates were produced by the same particle and are themselves neutral
//       particles like photons from a pi0 decay.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef ANA_CANDIDATEMATH_H
#define ANA_CANDIDATEMATH_H

//util includes
#include "util/units.h"
#include "util/mathWithUnits.h"

namespace ana
{
  //For all functions, CANDIDATE is a struct with these members:
  //1) transverse (distance from vertex)
  //2) z (absolute in MINERvA detector coordinate system)
  template <class CANDIDATE>
  mm DistFromVertex(const units::LorentzVector<mm>& vertex, const CANDIDATE& cand)
  {
    using namespace units;
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    return sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ));
  }

  template <class CANDIDATE>
  double CosineWrtMuon(const units::LorentzVector<mm>& vertex, const CANDIDATE& cand)
  {
    using namespace units;
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    return deltaZ.in<mm>() / sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ)).template in<mm>();
  }

  //Fraction of speed of light
  template <class CANDIDATE>
  double Beta(const units::LorentzVector<mm>& vertex, const CANDIDATE& cand)
  {
    return DistFromVertex(vertex, cand).template in<mm>() / cand.time.template in<ns>() / 300.; //Speed of light is 300mm/ns
  }

  //CANDIDATE must also have:
  //1) edep (energy deposited)
  template <class CANDIDATE>
  MeV InvariantMass(const units::LorentzVector<mm>& vertex, const CANDIDATE& lhs, const CANDIDATE& rhs)
  {
    using namespace units;
    const double lhsAngle = CosineWrtMuon(vertex, lhs);
    const double rhsAngle = CosineWrtMuon(vertex, rhs);
    return sqrt(2. * lhs.edep.template in<MeV>() * rhs.edep.template in<MeV>() * (1. - cos(acos(lhsAngle) + acos(rhsAngle))));
  }
}

#endif //ANA_CANDIDATEMATH_H
