//File: WithUnits.cpp
//Brief: A histogram WithUnits<> is constructed from any Fill()able object that can
//       have its title accessed via Get/SetTitle().  It can only be filled with values
//       related to a particular base unit, and its' title will automatically have units
//       added.  It should work with HistWrapper(2D)<> as well as THND.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNIT_WITHUNITS_CPP
#define UNIT_WITHUNITS_CPP

//unit library includes
#include "units/units.h"

namespace unit
{
  //HIST is any Fill()able object with GetXaxis() and related functions.
  //Examples could be a TH1D or a HistWrapper2D<>.
  //
  //AXES is a list of units.  If you Fill() your histogram with 2 numbers,
  //like a TH2D, you need 2 + 1 = 3 AXES: 2 for the binned axes and one for
  //the number of entries.  For example:
  //
  //WithUnits<HistWrapper<MnvH1D>, mm, candidates> hist("myHist", "A histogram;x axis", 10, 0, 1);
  //hist.Fill(3_mm); //Fill() with an implicit weight of 1 candidate
  //hist.Fill(3_mm, 2_candidates); //Fill() with a weight needs units!
  //hist.Fill(3_mm, 6_GeV); //Doesn't compile: wrong units for weight!
  //hist.Fill(3_mm, 6_candidates, 2_GeV); //Doesn't compile: too many arguments!
  //hist.Fill(3); //Doesn't compile: No units on the 3 -> error message!

  template <class HIST, class ...AXES>
  class WithUnits: public HIST
  {
    public:
      template <class ...ARGS>
      WithUnits(ARGS... args): HIST(args...)
      {
        SetAxisTitles<AXES...>();
      }

      virtual ~WithUnits() = default;

      //Force the user to use units for Fill()ing a histogram.
      //This implies that the user has to specify units for the "sum of weights" axis too.
      template <class ...ARGS>
      int Fill(ARGS... values)
      {
        //Print a sensible error message if there are more ARGS than AXES
        static_assert(sizeof...(AXES) - sizeof...(ARGS) == 1 || sizeof...(ARGS) == sizeof(AXES),
                      "You tried to Fill() a WithUnits<> with a number of arguments that doesn't match the number of axes!  "
                      "You can either Fill() omitting the weight axis (which is implicitly 1), or you can provide a weight.  "
                      "Maybe you forgot to provide units for the weight axis?  That's just asking for a denied approval package!");

        //Try to convert each of values... into the type of AXES...
        //Will fail at compile-time with incompatible units.
        return HIST::Fill(values.as<AXES>()...);
      }

      //TODO: Since I'm requiring that the "sum of weights" axis have units, overload Scale() too.

      //Forbid use of the base class's Fill() which does not enforce unit checking
      template <class ...ARGS>
      virtual int Fill(ARGS... bareNumbers) override
      {
        static_assert(false, "Filling a WithUnits<> with bare numbers is not allowed!  Fill it with the right units or just use a histogram directly.");
        return HIST::Fill(bareNumbers...);
      }

    private:
      //Implementation details to map number of AXES... to axis access functions
      template <class X>
      void SetAxisTitles()
      {
        GetXaxis()->SetTitle((std::string(GetXaxis())->GetTitle() + " [" + detail::unitName<X>() + "]").c_str());
      }

      template <class X, class Y>
      void SetAxisTitles()
      {
        GetYaxis()->SetTitle((std::string(GetYaxis()->GetTitle()) + " [" + detail::unitName<Y>() + "]").c_str());
        SetAxisTitles<X>();
      }

      template <class X, class Y, class Z>
      void SetAxisTitles()
      {
        GetZaxis()->SetTitle((std::string(GetZaxis()->GetTitle()) + " [" + detail::unitName<Z>() + "]").c_str());
        SetAxisTitles<X, Y>();
      }
  };

  //Metaprogramming to make axis labels work.
  //Should work with ROOT's LaTeX renderer.
  namespace detail
  {
    template <class UNIT>
    std::string unitName()
    {
      return UNIT::tag::name;
    }

    template <class LAST>
    std::string productName(std::string& appendTo)
    {
      return appendTo + LAST::tag::name;
    }

    template <class LHS, class NEXT, class ...PRODS>
    std::string productName(std::string& appendTo)
    {
      appendTo += LHS::name + " * ";
      return productName<NEXT, PRODS...>(appendTo);
    }

    template <class ...PRODS>
    std::string unitName()
    {
      std::string name;
      return productName<PRODS...>(name);
    }

    template <class ...NUM, class ...DENOM>
    std::string unitName()
    {
      //Write fractions in LaTeX that ROOT can understand
      std::string name = "%frac{";
      productName<NUM...>(name) + "}{";
      return productName<DENOM...>(name) + "}";
    }
  }
}

#endif //UNIT_WITHUNITS_CPP
