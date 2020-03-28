//File: DansPtRecoilWeight.cpp
//Brief: A DansPtRecoilWeight model modifies the CV's weight to account for a better-modeled or better-constrained
//       flux.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//models includes
#include "models/DansPtRecoilWeight.h"

//evt includes
#include "evt/CVUniverse.h"

namespace
{
  template <int exponent>
  double pow(const double base)
  {
    static_assert(exponent > 0, "exponents < 0 not implemented yet");
    return base*pow<exponent - 1>(base);
  }

  //Base case to end recursion
  template <>
  double pow<0>(const double /*base*/)
  {
    return 1;
  }
}

namespace model
{
  DansPtRecoilWeight::DansPtRecoilWeight(const YAML::Node& config): Model(config)
  {
  }

  events DansPtRecoilWeight::GetWeight(const evt::CVUniverse& univ) const
  {
    if((univ.GetTreeName() == "Truth") || (!hasTrueSingleChargedPion(univ) && !hasTrueSingleNeutralPion(univ))) return 1;

    const MeV recoil = univ.GetVecElem("recoil_summed_energy", 0);
    units::XYZVector<double> zHat{0., 0., 1.};
    const auto pt = univ.GetMuonP().p().cross(zHat).mag();

    //Comment from Dan from where I stole this code:
    //01-20-2020 new kevin fit
    return 1 + (-0.40352354995845363 + 7.567852048332732/std::exp(10.079490850299035*pow<2>(pt.in<GeV>())) -  0.30287822718991253*pt.in<GeV>())/std::exp((1250*pow<2>(recoil.in<GeV>()))/9.) + 0.9000000000000001*recoil.in<GeV>()*(0.5 + 0.1625/std::max(0.075, pt.in<GeV>()));
  }

  bool DansPtRecoilWeight::hasTrueSingleChargedPion(const evt::CVUniverse& univ) const
  {
    const auto pdgCodes = univ.GetVecInt("mc_FSPartPDG");
    int nChargedPi = 0, nPi = 0;
    for(const auto pdg: pdgCodes)
    {
      if(std::fabs(pdg) == 211)
      {
        ++nChargedPi;
        ++nPi;
      }
      else if(pdg == 111) ++nPi;
    }

    return (nPi == 1) && (nChargedPi == 1);
  }

  bool DansPtRecoilWeight::hasTrueSingleNeutralPion(const evt::CVUniverse& univ) const
  {
    const auto pdgCodes = univ.GetVecInt("mc_FSPartPDG");
    int nNeutralPi = 0, nPi = 0;
    for(const auto pdg: pdgCodes)
    {
      if(std::fabs(pdg) == 211) ++nPi;
      else if(pdg == 111)
      {
        ++nNeutralPi;
        ++nPi;
      }
    }
  
    return (nPi == 1) && (nNeutralPi == 1);
  }
}

namespace
{
  model::Model::Registrar<model::DansPtRecoilWeight> reg_flux("DansPtRecoilWeight");
}
