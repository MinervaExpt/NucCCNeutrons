//File: NeutronMultiplicityEAvailableCutStudy.cpp
//Brief: Plot truth neutron multiplicity in truth EAvailable to look for
//       a phase space where neutron multiplicity is changing rapidly.
//       I want to avoid such regions when I cut on EAvailable in the
//       1D multiplicity analysis.
//
//       It turns out that a CrossSection2DSignal<> already does that.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//studies includes
#include "analyses/studies/CrossSection2DSignal.h"
#include "analyses/studies/NeutronMultiplicity.cpp"
#include "analyses/studies/EAvailable.cpp"

#ifndef ANA_NEUTRONMULTIPLICITYEAVAILABLECUTSTUDY_CPP
#define ANA_NEUTRONMULTIPLICITYEAVAILABLECUTSTUDY_CPP

namespace
{
  static ana::CrossSection2DSignal<ana::NeutronMultiplicity, ana::EAvailable>::Registrar MultiplicityInEAvailableCut_reg("MultiplicityInEAvailableCut");
}

#endif //ANA_NEUTRONMULTIPLICITYEAVAILABLECUTSTUDY_CPP
