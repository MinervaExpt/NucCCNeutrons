//File: mathWithUnits.h
//Brief: C math functions like pow() and sqrt() that return the right units.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UTIL_MATHWITHUTILS_H
#define UTIL_MATHWITHUTILS_H

//util includes
#include "util/units.h"

//c++ includes
#include <cmath>

namespace units
{
  //Make the compiler try this before std::sqrt().  Ultimately, this should go in BaseUnits.
  template <class TAG, class PREFIX, class FLOAT>
  quantity<TAG, std::ratio<1>, FLOAT> sqrt(const quantity<productTag<TAG, TAG>, PREFIX, FLOAT> value)
  {
    return std::sqrt(value.template in<quantity<productTag<TAG, TAG>, std::ratio<1>, FLOAT>>());
  }

  namespace detail
  {
    //Trick to let me write a partial specialization for a function template:
    //wrap its behavior in a class instead
    template <int expo, class UNIT>
    struct do_pow
    {
      static_assert(expo > 1, "pow<>() not implemented for exponents < 0");
      using result_t = decltype(std::declval<UNIT>() * std::declval<typename do_pow<expo-1, UNIT>::result_t>());

      static result_t result(const UNIT value)
      {
        return value * do_pow<expo-1, UNIT>::result(value);
      }
    };

    template <class UNIT>
    struct do_pow<1, UNIT>
    {
      using result_t = UNIT;

      static result_t result(const UNIT value)
      {
        return value;
      }
    };
  }

  template <int expo, class UNIT>
  typename detail::do_pow<expo, UNIT>::result_t pow(const UNIT value)
  {
    return detail::do_pow<expo, UNIT>::result(value);
  }

  template <class TAG, class PREFIX, class FLOAT>
  FLOAT sin(const quantity<TAG, PREFIX, FLOAT> angle)
  {
    return std::sin(angle.template in<quantity<TAG, PREFIX, FLOAT>>());
  }

  template <class TAG, class PREFIX, class FLOAT>
  FLOAT cos(const quantity<TAG, PREFIX, FLOAT> angle)
  {
    return std::cos(angle.template in<quantity<TAG, PREFIX, FLOAT>>());
  }

  template <class TAG, class PREFIX, class FLOAT>
  FLOAT atan2(const quantity<TAG, PREFIX, FLOAT> x, const quantity<TAG, PREFIX, FLOAT> y)
  {
    return std::atan2(x.template in<quantity<TAG, PREFIX, FLOAT>>(), y.template in<quantity<TAG, PREFIX, FLOAT>>());
  }

  template <class TAG, class PREFIX, class FLOAT>
  quantity<TAG, PREFIX, FLOAT> fabs(const quantity<TAG, PREFIX, FLOAT> value)
  {
    return (value.template in<quantity<TAG, PREFIX, FLOAT>>() < 0)?-value:value;
  }
}

#endif //UTIL_MATHWITHUTILS_H
