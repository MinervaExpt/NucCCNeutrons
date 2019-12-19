//File: units.h
//Brief: A library for associating numbers with units at compile-time.
//       It makes the compiler check the unit-correctness of your
//       math for you!
//
//       Include this header only to use this library!
//
//       Some highlights:
//       *) user-defined units
//       *) compiler will refuse to add quantity<>s with different base units
//       *) unit names when printing quantity<>s
//       *) automatic unit conversions when adding units related by a prefix
//       *) unit conversions are kept track of by the compiler until you actually need the result
//       *) derived units, like MeV/cm, have names automatically generated as needed
//       *) you can always get a regular number back out of this interface: just use number.in<UNIT_NAME>()
//       *) all prefixed units stored in a user-defined base unit
//
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Numbers in some unit are represented by this library as quantity<>s.  quantity<>s can be added, subtracted, multiplied,
//divided, and negated like regular doubles.  There are a few ways to construct a quantity<>, called entry points:
//
//*) Directly: const MeV protonMass = 938.3
//*) As a "literal": const auto protonEnergy = 1.034_GeV
//*) As a branch of a TTree: quantity<> is said to be "aligned like" its underlying type.
//   So, in the example below, you could SetBranchAddress() for a TTree branch that stores
//   doubles to the address of protonMass directly.
//   The same should be true if you're using some C API like OpenGL.
//
//At all entry points, you are asserting that you got the names of units correct!  The compiler should take it
//from there until you reach an exit point:
//
//*) quantity<>::in<>(): protonMass.in<MeV>()
//*) Writing to a TTree.  See the last entry point.
//
//Exit points from the units library are useful for interfacing with libraries like ROOT that don't know about
//this library.  I recommend you use the units library for as much of your application as possible to get maximum
//benefit.
//
//The user defined a system of units via one of 3 macros provided:
//
//*) DECLARE_UNIT(): A quantity<> for a base unit that stores numbers as doubles.  Good starting point.
//*) DECLARE_RELATED_UNIT(): Define a unit related by a prefix or conversion to another unit.  Use this to
//   describe automatic units conversions to the compiler.
//*) DECLARE_UNIT_WITH_TYPE(): Like DECLARE_UNIT(), but more flexible by using a type other than double.
//
//If you call these macros in a namespace, like:
//
//namespace mnv
//{
//  DECLARE_UNIT(MeV)
//}
//
//Then the types you generate should be in that namespace:
//
//mnv::MeV test = 5000;
//
//Declaring your units in a namespace seems to me like a good way to avoid clashes with things like CLHEP.
//Of course, you can declare your own tags and implicit conversions.  I explain how to do that at the bottom
//of quantity.h.

#ifndef UNITS_UNITS_H
#define UNITS_UNITS_H

//units includes
#include "derivedUnits.h"
#include "quantity.h"
#include "printUnits.h"
#include "macros.h"

//Example snippet of a program using this library:
//
////Set up a system of units based on MeV.  GeV are related to MeV by a prefix.
//DECLARE_UNIT(MeV)
//DECLARE_RELATED_UNIT(GeV, MeV, 1000, 1)
//
////Set up another, unrelated, system of units.  cm can be added to mm, but not
////to MeV or GeV.
//DECLARE_UNIT(cm)
//DECLARE_RELATED_UNIT(mm, cm, 1, 10)
//
////You don't have to use doubles with units
//DECLARE_UNIT_WITH_TYPE(events, int)
//
////Do math with MeV
//const MeV protonMass = 938.3;
//const auto protonEnergy = 1.034_GeV; //literal value based on the DECLARE_RELATED_UNIT() macro above
//
////MeV and GeV are compatible.  The compiler keeps track of the conversion. 
//const auto ke = protonEnergy - protonMass;
//  
////Of course, ROOT wouldn't like it very much if you gave it an MeV.
////So, do this:
//yourHistogram.Fill(ke.in<MeV>());
//  
////If you multiply or divide two quantity<>s, the compiler
////will generate a new name for the resulting derived units
////for you.
//cm dx = 10.;
//const auto dEdx = (protonEnergy - protonMass)/dx;
// 
////quantity<>s get printed with their unit names.
//std::cout << "protonMass is " << protonMass << "\n";
//std::cout << "protonEnergy is " << protonEnergy << "\n";
//std::cout << "ke is " << ke << "\n";
//std::cout << "dEdx is " << dEdx << "\n";
//std::cout << "ke + protonMass is " << ke + protonMass << "\n";
//
////You can request a quantity<> in specific units like the histogram example above.
////This usually does some multiplication and division while the program is running.
//std::cout << "Proton energy, which is " << protonEnergy << ", is " << protonEnergy.in<MeV>() << " in MeV\n";
//
////See test_units.cpp for more examples.

#endif //UNITS_UNITS_H
