//File: quantity.h
//Brief: A quantity<> is a number with associated units.
//       It makes the compiler check the unit-correctness of your
//       math for you!  Tags are nasty, so there's some macros at
//       the bottom of this file to help you define your own unit
//       system.  See units.h for a condensed tutorial.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//A quantity is a number in units of BASE_TAGs.  It had better be aligned (i.e. take exactly the same amount of memory)
//as FLOATING_POINT (double by default).  So, you should be able to put doubles from a ROOT TTree into a quantity<MeV>
//for example.
//
//To use quantity<>, you at least need to define a "tag" for the unit you want to express.  BASE_TAG needs to have exactly
//one member: a constexpr char[] with the name you want printed with this unit.  BASE_TAG is also the only way to uniquely
//identify related units.  If you want to define a shortcut that relates one unit to another, you need to set the second
//template parameter to a ratio of new unit to BASE_TAG.  See examples at the bottom of this file.
//
//Inspired by how std::chrono::duration in the c++11 standard library works.  std::ratio<> relates one unit to another.

#ifndef UNITS_QUANTITY_H
#define UNITS_QUANTITY_H

//c++ includes
#include <ratio>

//Technical overview of quantity<>:
//On to the core of this library: quantity<>.  A quantity<> is a number counted in BASE_TAGs.
//BASE_TAG is the name of a unit, and quantity<> associates a number with it.  quantity<> is
//guaranteed to be aligned the same as its underlying FLOATING_POINT representation.  A given
//quantity<> can actually represent a metric prefix applied to BASE_TAG in the form of PREFIX.

namespace units
{
  //Details needed to ensure that conversions to the base unit are free
  namespace detail
  {
    template <class CONVERSION, class FLOATING_POINT>
    struct conversion
    { 
      static inline FLOATING_POINT do_convert(const FLOATING_POINT value)
      { 
        return value * CONVERSION::num / CONVERSION::den;
      }
    };
  
    //Specialization for CONVERSION = std::ratio<1>: No multiplication needed!
    template <class FLOATING_POINT>
    struct conversion<std::ratio<1>, FLOATING_POINT>
    { 
      static inline FLOATING_POINT do_convert(const FLOATING_POINT value)
      { 
        return value;
      }
    };
  }
  
  //TODO: Compatibility with quantities<> that have FLOATING_POINT types convertible to this one.
  //      There's no technical problem.  I just don't want to type all of that code.  Realized that
  //      this requires changes to buildProduct<> too.
  template <class BASE_TAG, class PREFIX=std::ratio<1>, class FLOATING_POINT = double>
  class quantity
  {
    public:
      //typedefs to help out macros
      using tag = BASE_TAG;
      using floating_point = FLOATING_POINT;
      using prefix = PREFIX;
  
      //Convert this quantity<> to another prefix with the same BASE_TAG.
  
      //At first glance, it seems like I would want this to return OTHER_QUANTITY.  However, OTHER_QUANTITY is
      //already compatible with this class and already converts to a value() in the same units as this class.
      //So, there's no reason to produce an OTHER_QUANTITY at all.
  
      //For the future: I could imagine using in<>() to change the design of this entire library:
      //                I could perform explicit conversions in all arithmetic operators and give
      //                each prefix its own name instead of forcing everything to work in one base
      //                unit.  I'd still need some BASE_TAG in addition to a real tag with name to
      //                use.  PREFIX would only be used to relate one prefixed unit to another.
      //                All I'm missing right now is unit names for prefixed quantity<>s.  Maybe I
      //                could even do something crazy like specialize a class template for std::milli.
  
      //TODO: Don't do multiplication if OTHER_QUANTITY happens to have a PREFIX of std::ratio<1>!
      //      I'll probably need a struct template to allow for the specializations I need.
      template <class OTHER_QUANTITY>
      FLOATING_POINT in() const
      {
        static_assert(std::is_same<typename OTHER_QUANTITY::tag, BASE_TAG>::value, "You cannot convert quantities with different base units!");
        return detail::conversion<std::ratio_divide<PREFIX, typename OTHER_QUANTITY::prefix>, FLOATING_POINT>::do_convert(fValue);
      }
  
      //Construct a quantity from a FLOATING_POINT.  Your entry point
      //to a compiler-enforced unit system.
      quantity(const FLOATING_POINT value): fValue(value) {}
  
      //Construct a quantity<> from another quantity<> related to BASE_TAG by a ratio<>.
      template <class OTHER_PREFIX>
      quantity(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other): fValue(other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>()) {}
  
      //Addition and subtraction only make sense with other quantities that have the same BASE_TAG
      template <class OTHER_PREFIX>
      quantity<BASE_TAG, PREFIX, FLOATING_POINT> operator +(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other) const
      {
        return quantity<BASE_TAG, PREFIX, FLOATING_POINT>(fValue + other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>());
      }
  
      template <class OTHER_PREFIX>
      quantity<BASE_TAG, PREFIX, FLOATING_POINT>& operator +=(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other)
      {
        *this = *this + other;
        return *this;
      }
  
      template <class OTHER_PREFIX>
      quantity<BASE_TAG, PREFIX, FLOATING_POINT> operator -(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other) const
      {
        return quantity<BASE_TAG, PREFIX, FLOATING_POINT>(fValue - other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>());
      }
  
      template <class OTHER_PREFIX>
      quantity<BASE_TAG, PREFIX, FLOATING_POINT>& operator -=(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other)
      {
        *this = *this - other;
        return *this;
      }
  
      //Negation operator will fail to compile if FLOATING_POINT happens to be unsigned.
      typename std::enable_if<std::is_signed<FLOATING_POINT>::value, quantity<BASE_TAG, PREFIX, FLOATING_POINT>>::type operator -() const
      {
        return quantity(-fValue);
      }
  
      //When multiplying or dividing quantities, automatically generate a tag for derived units
      template <class RHS_UNIT, class OTHER_PREFIX>
      quantity<typename buildProduct<BASE_TAG, RHS_UNIT>::result, std::ratio_multiply<PREFIX, OTHER_PREFIX>, FLOATING_POINT> operator *(const quantity<RHS_UNIT, OTHER_PREFIX, FLOATING_POINT> rhs) const
      {
        return quantity<typename buildProduct<BASE_TAG, RHS_UNIT>::result, std::ratio_multiply<PREFIX, OTHER_PREFIX>, FLOATING_POINT>(fValue * rhs.template in<quantity<RHS_UNIT, OTHER_PREFIX, FLOATING_POINT>>());
      }
  
      template <class RHS_UNIT, class OTHER_PREFIX>
      quantity<typename buildRatio<BASE_TAG, RHS_UNIT>::result, std::ratio_divide<PREFIX, OTHER_PREFIX>, FLOATING_POINT> operator /(const quantity<RHS_UNIT, OTHER_PREFIX, FLOATING_POINT> rhs) const
      {
        return quantity<typename buildRatio<BASE_TAG, RHS_UNIT>::result, std::ratio_divide<PREFIX, OTHER_PREFIX>, FLOATING_POINT>(fValue / rhs.template in<quantity<RHS_UNIT, OTHER_PREFIX, FLOATING_POINT>>());
      }
  
      //Comparison operators
      template <class OTHER_PREFIX>
      bool operator <(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other) const
      {
        return fValue < other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>();
      }
  
      template <class OTHER_PREFIX>
      bool operator >(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other) const
      {
        return fValue > other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>();
      }
  
      template <class OTHER_PREFIX>
      bool operator ==(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other) const
      {
        return fValue == other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>();
      }
  
      template <class OTHER_PREFIX>
      bool operator !=(const quantity<BASE_TAG, OTHER_PREFIX, FLOATING_POINT> other) const
      {
        return fValue != other.template in<quantity<BASE_TAG, PREFIX, FLOATING_POINT>>();
      }
  
    private:
      //TODO: A preprocessor macro to hide the private above in case users need to hack their way past my unit constraints?
  
      //Does this need to be public to allow TTree to write to it?  That doesn't seem to be the case for GenVector which has
      //private vector components and I use in my analysis.
      FLOATING_POINT fValue;
  };
}
#endif //UNITS_QUANTITY_H
