//File: test_units.cpp
//Brief: Test of my header-only unit system library.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//The library I want to test: Does compile-time unit checking
#include "units.h"

//c++ includes
#include <iostream>
#include <functional> //std::less

DECLARE_UNIT(MeV)
DECLARE_RELATED_UNIT(GeV, MeV, 1000, 1)

DECLARE_UNIT(cm)
DECLARE_RELATED_UNIT(mm, cm, 1, 10)

DECLARE_UNIT_WITH_TYPE(events, int)

int main(const int /*argc*/, const char** /*argv*/)
{
  //Do math with MeV
  const MeV protonMass = 938.3;
  const auto protonEnergy = 1.034_GeV; //literal value example
  
  const auto ke = protonEnergy - protonMass;
  
  //Of course, ROOT wouldn't like it very much if you gave it an MeV.
  //So, do this:
  //yourHistogram.Fill(ke.value());
  
  //Test derived units
  cm dx = 10.;
  const auto dEdx = (protonEnergy - protonMass)/dx;
  
  //Test unit name printout
  std::cout << "protonMass is " << protonMass << "\n";
  std::cout << "protonEnergy is " << protonEnergy << "\n";
  std::cout << "ke is " << ke << "\n";
  std::cout << "dEdx is " << dEdx << "\n";
  std::cout << "ke + protonMass is " << ke + protonMass << "\n";

  //Test modifying operators
  std::cout << "dx started as " << dx << "\n";
  dx -= 5_cm;
  std::cout << "After modification, dx is " << dx << "\n";
  dx += 15_cm;
  std::cout << "I added " << 15_cm << " to dx: " << dx << "\n"; 
  std::cout << "I've negated dx: " << -dx << "\n";

  //Test comparison operators
  const auto otherDx = 30_cm;
  std::cout << dx << " < " << otherDx << "?  " << std::boolalpha << (dx < otherDx) << "\n";
  std::cout << dx << " < " << 11_cm << "?  " << std::boolalpha << (dx < 11_cm) << "\n";
  const auto thirdDistance = 390_mm;
  std::cout << thirdDistance << " > " << otherDx << "?  " << std::boolalpha << (thirdDistance > otherDx) << "\n";
  std::cout << thirdDistance << " > " << 401_mm << "?  " << std::boolalpha << (thirdDistance > 401_mm) << "\n";

  const auto integral = 987_events; //A unit that only comes in integers.  Easier for comparison to make sense.
  const auto anotherCount = integral;
  std::cout << integral << " == " << anotherCount << "?  " << std::boolalpha << (integral == anotherCount) << "\n";
  std::cout << integral << " != " << 988_events << "?  " << std::boolalpha << (integral != 988_events) << "\n";

  //Also works with c++ standard library
  std::cout << "The smaller of " << dx << " and " << otherDx << " is " << std::min(dx, otherDx) << "\n";

  //Test generalized productTag<>s
  const auto prod = dx*ke*thirdDistance*dx*dx;
  std::cout << "Printing a product of 5 types: " << prod << "\n";
  std::cout << "The answer I get by hand is: " << 20*(1034 - 938.3)*39*20*20 << "\n";
  
  //Test explicit unit conversions
  std::cout << "Proton mass, which is " << protonMass << ", is " << protonMass.in<GeV>() << " in GeV\n";
  std::cout << "Proton energy, which is " << protonEnergy << ", is " << protonEnergy.in<MeV>() << " in MeV\n";
  std::cout << "Proton mass, which is " << protonMass << ", is " << protonMass.in<MeV>() << " in MeV\n";
  std::cout << "Proton energy, which is " << protonEnergy << ", is " << protonEnergy.in<GeV>() << " in GeV\n";

  //These lines of code shoulnd't compile:
  /*const auto mixUnits = dx + ke;
  const auto mixDerived = ke - dEdx;
  dx.in<GeV>();*/

  //Test more complicated derived units
  std::cout << dEdx << " over " << prod << " is " << dEdx/prod << "\n";
  const auto anotherRatio = protonEnergy/otherDx;
  std::cout << anotherRatio << " over " << dEdx << " is " << anotherRatio / dEdx << "\n";
  std::cout << anotherRatio << " times " << dEdx << " is " << anotherRatio * dEdx << "\n";
  std::cout << "Product of 3 derived types: " << anotherRatio * dEdx * prod << "\n";

  return 0;
}
