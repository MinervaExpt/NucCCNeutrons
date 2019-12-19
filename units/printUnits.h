//File: printUnits.h
//Brief: Make quantity<>s printable with std::cout and relatives.
//       Implemented as global overloads for operator <<().  Has
//       special handling for derived units.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNITS_PRINTUNITS_H
#define UNITS_PRINTUNITS_H

//c++ includes
#include <iostream>

namespace units
{
  //Nota Bene: Since BASE_TAG::name must be matched to the base unit by the compiler, I always want to
  //           convert to the base unit for printing.  That's why in<>() seems to ignore PREFIX only
  //           in the following operator << overloads.
  template <class BASE_TAG, class PREFIX, class FLOATING_POINT>
  std::ostream& operator <<(std::ostream& os, const quantity<BASE_TAG, PREFIX, FLOATING_POINT> value)
  {
    return os << value.template in<quantity<BASE_TAG, std::ratio<1>, FLOATING_POINT>>() << " " << BASE_TAG::name;
  }
  
  //Specialization to end recursion without a trailing *
  template <class LAST>
  std::ostream& printProduct(std::ostream& os)
  {
    return os << LAST::name;
  }
  
  //Helper function to print a productTag<>.
  //Note to self: NEXT is just there to force the 1-template-parameter overload to be chosen.
  template <class LHS, class NEXT, class ...PRODS>
  std::ostream& printProduct(std::ostream& os)
  {
    os << LHS::name << " * ";
    return printProduct<NEXT, PRODS...>(os);
  }
  
  //Specialization to print out a quantity<> with a productTag<> or a ratioTag<>
  template <class ...PRODS, template <class ...> class PRODUCT, class PREFIX, class FLOATING_POINT>
  std::ostream& operator <<(std::ostream& os, const quantity<PRODUCT<PRODS...>, PREFIX, FLOATING_POINT> value)
  {
    os << value.template in<quantity<PRODUCT<PRODS...>, std::ratio<1>, FLOATING_POINT>>() << " ";
    return printProduct<PRODS...>(os);
  }
  
  template <class ...NUM, class ...DENOM, class PREFIX, class FLOATING_POINT>
  std::ostream& operator <<(std::ostream& os, const quantity<ratioTag<productTag<NUM...>, productTag<DENOM...>>, PREFIX, FLOATING_POINT> value)
  {
    os << value.template in<quantity<ratioTag<productTag<NUM...>, productTag<DENOM...>>, std::ratio<1>, FLOATING_POINT>>() << " (";
    printProduct<NUM...>(os) << ") / (";
    return printProduct<DENOM...>(os) << ")";
  }
}

#endif //UNITS_PRINTUNITS_H
