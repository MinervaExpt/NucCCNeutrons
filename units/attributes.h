//File: attributes.h
//Brief: Associate attributes, like unit name, to unit types.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNITS_ATTRIBUTES_H
#define UNITS_ATTRIBUTES_H

namespace units
{
  //Empty attributes for types not registered.
  //This struct will be filled out in unit macros.
  template <class UNIT>
  struct attributes
  {
    //static constexpr auto name = "someUnit";
  };
}

#endif //UNITS_ATTRIBUTES_H
