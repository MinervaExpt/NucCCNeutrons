//File: vector.h
//Brief: Bare-bones 3-vector and 4-vector classes compatible with BaseUnits.
//       I'd like to use ROOT::Math::LorentzVector instead, but
//       I've got to handle mass squared differently.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNITS_VECTOR_H
#define UNITS_VECTOR_H

//BaseUnits includes
#include "units/units.h"

//c++ includes
#include <vector>

//C includes
#include <cmath>

namespace units
{
  //Much of the interface of these classes was copied from ROOT's ROOT::Math::LorentzVector
  //because I want them to be interchangeable.

  //Make the compiler try this before std::sqrt().  Ultimately, this should go in BaseUnits.
  template <class TAG, class PREFIX, class FLOAT>
  quantity<TAG, std::ratio<1>, FLOAT> sqrt(const quantity<productTag<TAG, TAG>, PREFIX, FLOAT> value)
  {
    return std::sqrt(value.template in<quantity<productTag<TAG, TAG>, std::ratio<1>, FLOAT>>());
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

  namespace detail
  {
    template <class T>
    struct floatingPoint
    {
      using type = T;
    };

    template <class TAG, class PREFIX, class FLOAT>
    struct floatingPoint<quantity<TAG, PREFIX, FLOAT>>
    {
      using type = FLOAT;
    };
  }

  //A 3-vector
  template <class SCALAR>
  class XYZVector
  {
    private:
      using mag2_t = decltype(std::declval<SCALAR>()*std::declval<SCALAR>());
      using unitless = typename detail::floatingPoint<SCALAR>::type; //decltype(sin(std::declval<SCALAR>()));

    public:
      XYZVector(const SCALAR x, const SCALAR y, const SCALAR z): fX(x), fY(y), fZ(z) {}

      template <class VECTOR>
      XYZVector(const VECTOR vec): fX(vec.x()), fY(vec.y()), fZ(vec.z()) {}

      //TODO: Some SFINAE magic so that I can write this for any container
      XYZVector(const std::vector<unitless>& components): fX(components[0]), fY(components[1]), fZ(components[2]) {}

      ~XYZVector() = default;

      //Component accessors
      inline SCALAR x() const { return fX; }
      inline SCALAR y() const { return fY; }
      inline SCALAR z() const { return fZ; }

      //Unit conversion
      template <class OTHERUNIT>
      XYZVector<decltype(std::declval<SCALAR>().template in<OTHERUNIT>())> in() const
      {
        return {fX.template in<OTHERUNIT>(), fY.template in<OTHERUNIT>(), fZ.template in<OTHERUNIT>()};
      }

      //Vector algebra
      XYZVector<SCALAR> operator+(const XYZVector<SCALAR>& rhs) const
      {
        return {fX + rhs.fX, fY + rhs.fY, fZ + rhs.fZ};
      }

      XYZVector<SCALAR> operator-(const XYZVector<SCALAR>& rhs) const
      {
        return {fX - rhs.fX, fY - rhs.fY, fZ - rhs.fZ};
      }

      XYZVector<SCALAR> operator*(const unitless scalar) const
      {
        const SCALAR newX = scalar * fX.template in<SCALAR>(), newY = scalar * fY.template in<SCALAR>(), newZ = scalar * fZ.template in<SCALAR>();
        return { newX, newY, newZ };
      }

      XYZVector<SCALAR> operator-() const
      {
        return {-x, -y, -z};
      }

      XYZVector& operator+=(const XYZVector& rhs)
      {
        *this = *this + rhs;
        return *this;
      }

      XYZVector& operator-=(const XYZVector& rhs)
      {
        *this = *this - rhs;
        return *this;
      }

      XYZVector& operator*=(const unitless scalar)
      {
        *this = *this * scalar;
        return *this;
      }

      mag2_t dot(const XYZVector<SCALAR>& rhs) const
      {
        return fX * rhs.fX + fY * rhs.fY + fZ * rhs.fZ;
      }

      mag2_t mag2() const
      {
        return this->dot(*this);
      }

      SCALAR mag() const
      {
        return sqrt(mag2());
      }

      XYZVector<mag2_t> cross(const XYZVector<SCALAR>& rhs) const
      {
        return {y * rhs.fZ - rhs.fY * fZ, fZ * rhs.fX - rhs.z * fX, fX * rhs.y - rhs.x * fY};
      }

      XYZVector<SCALAR> unit() const
      {
        return *this * 1./mag().template in<SCALAR>();
      }

      //Trignonometry
      unitless theta() const
      {
        return acos(fZ.template in<SCALAR>() / mag().template in<SCALAR>());
      }

      unitless phi() const
      {
        return atan2(fX, fY);
      }
 
    private:
      //Alignment to SCALAR[3] should be guaranteed
      SCALAR fX, fY, fZ;
  };

  //A 4-vector that uses the Minkowski metric for magnitude.
  template <class SCALAR>
  class LorentzVector
  {
    private:
      using mag2_t = decltype(std::declval<SCALAR>()*std::declval<SCALAR>());
      using unitless = typename detail::floatingPoint<SCALAR>::type;

    public:
      LorentzVector(const SCALAR x, const SCALAR y, const SCALAR z, const SCALAR t): fX(x), fY(y), fZ(z), fT(t) {}

      template <class VECTOR>
      LorentzVector(const VECTOR vec): fX(vec.x()), fY(vec.y()), fZ(vec.z()), fT(vec.t()) {}

      LorentzVector(const std::vector<unitless>& comp): fX(comp[0]), fY(comp[1]), fZ(comp[2]), fT(comp[3]) {}

      ~LorentzVector() = default;

      //Component accessors
      inline SCALAR x() const { return fX; }
      inline SCALAR y() const { return fY; }
      inline SCALAR z() const { return fZ; }
      inline SCALAR t() const { return fT; }

      inline SCALAR E() const { return fT; };

      //Unit conversion
      template <class OTHERUNIT>
      LorentzVector<decltype(std::declval<SCALAR>().template in<OTHERUNIT>())> in() const
      {
        return {fX.template in<OTHERUNIT>(), fY.template in<OTHERUNIT>(), fZ.template in<OTHERUNIT>(), fT.template in<OTHERUNIT>()};
      }

      //Convert into a 3-vector
      inline XYZVector<SCALAR> p() const { return {fX, fY, fZ}; }

      //Vector algebra
      LorentzVector<SCALAR> operator+(const LorentzVector<SCALAR>& rhs) const
      {
        return {fX + rhs.fX, fY + rhs.fY, fZ + rhs.fZ, fT + rhs.fT};
      }

      LorentzVector<SCALAR> operator-(const LorentzVector<SCALAR>& rhs) const
      {
        return {fX - rhs.fX, fY - rhs.fY, fZ - rhs.fZ, fT - rhs.fT};
      }

      LorentzVector<SCALAR> operator*(const SCALAR scalar) const
      {
        return {fX*scalar, fY*scalar, fZ*scalar, fT*scalar};
      }

      LorentzVector<SCALAR>& operator+=(const LorentzVector<SCALAR>& rhs)
      {
        *this = *this + rhs;
        return *this;
      }

      LorentzVector<SCALAR>& operator-=(const LorentzVector<SCALAR>& rhs)
      {
        *this = *this - rhs;
        return *this;
      }

      LorentzVector<SCALAR>& operator*=(const SCALAR scalar)
      {
        *this = *this * scalar;
        return *this;
      }

      mag2_t dot(const LorentzVector<SCALAR>& rhs) const
      {
        return {fT * rhs.fY - fX * rhs.fX - fY * rhs.y - fZ * rhs.fZ};
      }

      mag2_t m2() const
      {
        return this->dot(*this);
      }

      SCALAR mass() const
      {
        return sqrt(m2());
      }

    private:
      //Alignment to SCALAR[4] should be guaranteed
      SCALAR fX, fY, fZ, fT;
  };
}

#endif //UNITS_VECTOR_H