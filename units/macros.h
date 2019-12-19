//File: macros.h
//Brief: Avoid defining your own tags and get literals "for free" by using
//       these macros to define quantity<>-based types.  You can avoid using
//       macros entirely by following my advice at the end of this file.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNITS_MACROS_H
#define UNITS_MACROS_H

//Macro to make unit declaration less error-prone.  Adds some bells and whistles like user-defined literals.
#define DECLARE_UNIT_WITH_TYPE(unitName, type)\
  struct unitName##Tag\
  {\
    static constexpr auto name = #unitName;\
  };\
\
  /*Give this unit a convenient name.*/\
  using unitName = units::quantity<unitName##Tag, std::ratio<1>, type>;\
  /*Ensure that this unit is aligned like double as advertised*/\
  static_assert(sizeof(unitName) == sizeof(type), "Alignment of " #unitName " doesn't match alignment of a " #type "!");\
\
  /*Allow user literals for this unit*/\
  unitName operator "" _##unitName(const long double value)\
  {\
    return unitName(value);\
  }\
\
  unitName operator "" _##unitName(const unsigned long long int value)\
  {\
    return unitName(value);\
  }\
//End of DECLARE_UNIT_WITH_TYPE macro

//Macro to make unit declaration less painful for most cases.  You can always do it manually too.
#define DECLARE_UNIT(unitName)\
  DECLARE_UNIT_WITH_TYPE(unitName, double)
//End of DECLARE_UNIT_WITH_TYPE macro

//Macro to declare a unit related to another unit, like name = cm, relative = meters, num = 1, denom = 100
#define DECLARE_RELATED_UNIT(unitName, relative, num, denom)\
  /*Give the new unit a useful name*/\
  using unitName = units::quantity<relative::tag, std::ratio<num, denom>, relative::floating_point>;\
\
  /*Allow user literals for this unit*/\
  unitName operator "" _##unitName(const long double value)\
  {\
    return unitName(value);\
  }\
\
  unitName operator "" _##unitName(const unsigned long long int value)\
  {\
    return unitName(value);\
  }\
//End of DECLARE_RELATED_UNIT macro

//Example manual usage.  The macros do this for you.
//First, define the name of this type of unit
/*struct MeVTag
{
  static constexpr auto name = "MeV";
};

//Next, define a useful name for this unit
using MeV = units::quantity<MeVTag>;
static_assert(sizeof(MeV) == sizeof(double), "Alignment of MeV doesn't match the underlying floating point type!");

//Allow "literal" MeV values.  Look for example below.
MeV operator "" _MeV(const long double value)
{
  return MeV(value);
}

//Now, define a metric prefix related to this unit

//It would be cool to use std::giga, but I don't
//really want to provide results in eV.  Better to
//define my own unit prefix abbreviations.
using GeV = units::quantity<MeVTag, std::ratio<1000>>;

//Allow "literal" GeV values.  Look for example below.
GeV operator "" _GeV(const long double value)
{
  return GeV(value);
}*/

#endif //UNITS_MACROS_H
