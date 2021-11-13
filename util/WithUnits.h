//File: WithUnits.cpp
//Brief: A histogram WithUnits<> is constructed from any Fill()able object that can
//       have its title accessed via Get/SetTitle().  It can only be filled with values
//       related to a particular base unit, and its' title will automatically have units
//       added.  It should work with HistWrapper(2D)<> as well as THND.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNIT_WITHUNITS_CPP
#define UNIT_WITHUNITS_CPP

//PlotUtils includes so I can specialize on HistWrapper
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#include "PlotUtils/Model.h"

//unit library includes
#include "units/units.h"

namespace units
{
  //Metaprogramming to make axis labels work.
  //Should work with ROOT's LaTeX renderer.
  namespace detail
  {
    //Parse a UNIT's name into LaTeX
    template <class UNIT>
    struct unit
    {
      static std::string name()
      {
        return units::attributes<UNIT>::name;
      }
    };

    template <class LHS, class ...PRODS>
    struct product
    {
      static std::string name(std::string& appendTo)
      {
        appendTo += units::attributes<LHS>::name + " * ";
        return product<PRODS...>::name(appendTo);
      }
    };

    template <class LAST>
    struct product<LAST>
    {
      static std::string name(std::string& appendTo)
      {
        return appendTo + units::attributes<LAST>::name;
      }
    };

    //TODO: This doesn't work unfortunately.  How do I know whether units::quantity<productTag<GeV, GeV>, std::ratio<1, 1000000>, double> is MeV * MeV or keV * TeV?
    //      Just avoid putting derived units on axes for now :(
    template <class RATIO, class FLOAT_TYPE, class ...PRODS>
    struct unit<units::quantity<productTag<PRODS...>, RATIO, FLOAT_TYPE>>
    {
      static std::string name()
      {
        std::string name;
        return product<PRODS...>::name(name);
      }
    };

    template <class ...NUM, class ...DENOM>
    struct unit<ratioTag<productTag<NUM...>, productTag<DENOM...>>>
    {
      static std::string name()
      {
        //Write fractions in LaTeX that ROOT can understand
        std::string name = "%frac{";
        product<NUM...>::name(name) + "}{";
        return product<DENOM...>::name(name) + "}";
      }
    };
  }

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
      WithUnits(const WithUnits<HIST, AXES...>& other) = default;

      template <class ...ARGS>
      WithUnits(ARGS... args): HIST(args...)
      {
        //TODO: SetAxisLimits<AXES...>() to let me just pass std::vector<UNIT>
        SetAxisTitles<AXES...>();
      }

      virtual ~WithUnits() = default;

      //TODO: This Fill() function only works with a weight (sizeof...(AXES) must be same as sizeof...(ARGS)).
      //Force the user to use units for Fill()ing a histogram.
      //This implies that the user has to specify units for the "sum of weights" axis too.
      template <class ...ARGS>
      int Fill(ARGS... values)
      {
        //Print a sensible error message if there are more ARGS than AXES
        static_assert(sizeof...(AXES) - sizeof...(ARGS) == 1 || sizeof...(ARGS) == sizeof...(AXES),
                      "You tried to Fill() a WithUnits<> with a number of arguments that doesn't match the number of axes!  "
                      "You can either Fill() omitting the weight axis (which is implicitly 1), or you can provide a weight.  "
                      "Maybe you forgot to provide units for the weight axis?  That's just asking for a denied approval package!");

        //Try to convert each of values... into the type of AXES...
        //Will fail at compile-time with incompatible units.
        return HIST::Fill(values.template in<AXES>()...);
      }

      //TODO: Since I'm requiring that the "sum of weights" axis have units, overload Scale() too.

      void SetDirectory(TDirectory* dir)
      {
        HIST::SetDirectory(dir);
      }

    private:
      //Implementation details to map number of AXES... to axis access functions
      template <class X>
      void SetAxisTitles()
      {
        HIST::GetXaxis()->SetTitle((std::string(HIST::GetXaxis()->GetTitle()) + " [" + detail::unit<X>::name() + "]").c_str());
      }

      template <class X, class Y>
      void SetAxisTitles()
      {
        HIST::GetYaxis()->SetTitle((std::string(HIST::GetYaxis()->GetTitle()) + " [" + detail::unit<Y>::name() + "]").c_str());
        SetAxisTitles<X>();
      }

      template <class X, class Y, class Z>
      void SetAxisTitles()
      {
        HIST::GetZaxis()->SetTitle((std::string(HIST::GetZaxis()->GetTitle()) + " [" + detail::unit<Z>::name() + "]").c_str());
        SetAxisTitles<X, Y>();
      }
  };

  //Specialization to use with HistWrapper<>
  template <class UNIV, class XUNIT, class YUNIT>
  class WithUnits<PlotUtils::HistWrapper<UNIV>, XUNIT, YUNIT>: public PlotUtils::HistWrapper<UNIV>
  {
    private:
      using Base_t = PlotUtils::HistWrapper<UNIV>;

    public:
      template <class ...ARGS>
      WithUnits(ARGS... args): PlotUtils::HistWrapper<UNIV>(args...)
      {
        Base_t::hist->GetXaxis()->SetTitle((std::string(Base_t::hist->GetXaxis()->GetTitle()) + " [" + detail::unit<XUNIT>::name() + "]").c_str());
        Base_t::hist->GetYaxis()->SetTitle((std::string(Base_t::hist->GetYaxis()->GetTitle()) + " [" + detail::unit<YUNIT>::name() + "]").c_str());
      }

      virtual ~WithUnits() = default;

      //Fill without a weight
      template <class OTHERX>
      int Fill(const UNIV* univ, const OTHERX value)
      {
        return Base_t::univHist(univ)->Fill(value.template in<XUNIT>());
      }

      //Fill with a weight
      template <class OTHERX, class OTHERY>
      int Fill(const UNIV* univ, const OTHERX value, const OTHERY weight)
      { 
        return Base_t::univHist(univ)->Fill(value.template in<XUNIT>(), weight.template in<YUNIT>()); 
      }

      //Given many IsVertical() (or otherwise identical) Universes, Fill() them all with only one bin lookup.
      //For variable-width binning only, this gets faster than calling Fill() in a loop because ROOT has to
      //do a binary search for each Universe.
      template <class OTHERX, class EVENT>
      int Fill(const std::vector<UNIV*>& univs, const OTHERX value, const PlotUtils::Model<UNIV>& model, const EVENT& evt)
      {
        assert(!univs.empty());
        const int whichBin = Base_t::univHist(univs.front())->FindBin(value.template in<XUNIT>());

        for(const auto univ: univs)
        {
          auto hist = Base_t::univHist(univ);
          const double weight = model.GetWeight(*univ, evt);
          hist->AddBinContent(whichBin, weight);

          const double err = hist->GetBinError(whichBin);
          const double newErr = err*err + weight*weight;
          hist->SetBinError(whichBin, newErr);
        }

        return whichBin;
      }

      void SetDirectory(TDirectory* dir)
      {
        Base_t::hist->SetDirectory(dir);
      }
  };


  //Specialization to use with Hist2DWrapper<>
  template <class UNIV, class XUNIT, class YUNIT, class ZUNIT>
  class WithUnits<PlotUtils::Hist2DWrapper<UNIV>, XUNIT, YUNIT, ZUNIT>: public PlotUtils::Hist2DWrapper<UNIV>
  {
    private:
      using Base_t = PlotUtils::Hist2DWrapper<UNIV>;

    public:
      template <class ...ARGS>
      WithUnits(ARGS... args): PlotUtils::Hist2DWrapper<UNIV>(args...)
      {
        Base_t::hist->GetXaxis()->SetTitle((std::string(Base_t::hist->GetXaxis()->GetTitle()) + " [" + detail::unit<XUNIT>::name() + "]").c_str());
        Base_t::hist->GetYaxis()->SetTitle((std::string(Base_t::hist->GetYaxis()->GetTitle()) + " [" + detail::unit<YUNIT>::name() + "]").c_str());
        Base_t::hist->GetZaxis()->SetTitle((std::string(Base_t::hist->GetZaxis()->GetTitle()) + " [" + detail::unit<ZUNIT>::name() + "]").c_str());
      }

      virtual ~WithUnits() = default;

      //Fill without a weight
      template <class OTHERX, class OTHERY>
      int Fill(const UNIV* univ, const OTHERX x, const OTHERY y)
      {
        return Base_t::univHist(univ)->Fill(x.template in<XUNIT>(), y.template in<YUNIT>());
      }

      //Fill with a weight
      template <class OTHERX, class OTHERY, class OTHERZ>
      int Fill(const UNIV* univ, const OTHERX x, const OTHERY y, const OTHERZ weight)
      { 
        return Base_t::univHist(univ)->Fill(x.template in<XUNIT>(), y.template in<YUNIT>(), weight.template in<ZUNIT>()); 
      }

      //Given many IsVertical() (or otherwise identical) Universes, Fill() them all with only one bin lookup.
      //For variable-width binning only, this gets faster than calling Fill() in a loop because ROOT has to
      //do a binary search for each Universe.
      template <class OTHERX, class OTHERY, class EVENT>
      int Fill(const std::vector<UNIV*>& univs, const OTHERX x, const OTHERY y, const PlotUtils::Model<UNIV>& model, const EVENT& evt)
      {
        assert(!univs.empty());
        const int whichBin = Base_t::univHist(univs.front())->FindBin(x.template in<XUNIT>(), y.template in<YUNIT>());
                                                                                                                           
        for(const auto univ: univs)
        {
          auto hist = Base_t::univHist(univ);
          const double weight = model.GetWeight(*univ, evt);
          hist->AddBinContent(whichBin, weight);
                                                                                                                           
          const double err = hist->GetBinError(whichBin);
          const double newErr = err*err + weight*weight;
          hist->SetBinError(whichBin, newErr);
        }

        return whichBin;
      }

      void SetDirectory(TDirectory* dir)
      {
        Base_t::hist->SetDirectory(dir);
      }
  };
}

#endif //UNIT_WITHUNITS_CPP
