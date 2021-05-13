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
  double CosineWrtZAxis(const units::LorentzVector<mm>& vertex, const CANDIDATE& cand)
  {
    using namespace units;
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    return deltaZ.in<mm>() / sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ)).template in<mm>();
  }

  template <class CANDIDATE>
  radians ThetaWrtZAxis(const units::LorentzVector<mm>& vertex, const CANDIDATE& cand)
  {
    using namespace units;
    const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
    return atan2(cand.transverse.template in<mm>(), deltaZ.in<mm>());
  }

  //Fraction of speed of light
  template <class CANDIDATE>
  double Beta(const units::LorentzVector<mm>& vertex, const CANDIDATE& cand)
  {
    return DistFromVertex(vertex, cand).template in<mm>() / cand.time.template in<ns>() / 300.; //Speed of light is 300mm/ns
  }

  //CANDIDATE must also have:
  //1) edep (energy deposited)
  //2) x3D (from 3D fit)
  //3) y3D (from 3D fit)
  template <class CANDIDATE>
  MeV InvariantMass(const units::LorentzVector<mm>& vertex, const CANDIDATE& lhs, const CANDIDATE& rhs)
  {
    using namespace units;
    const auto lhsDirToMuon = (vertex.p() - units::XYZVector<mm>(lhs.x3D, lhs.y3D, lhs.z)).unit();
    const auto rhsDirToMuon = (vertex.p() - units::XYZVector<mm>(rhs.x3D, rhs.y3D, rhs.z)).unit();
    return sqrt(2. * lhs.edep.template in<MeV>() * rhs.edep.template in<MeV>() * (1. - lhsDirToMuon.dot(rhsDirToMuon)));
  }
}

#endif //ANA_CANDIDATEMATH_H
