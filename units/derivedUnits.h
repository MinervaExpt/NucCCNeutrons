//File: derivedUnits.h
//Brief: Rules for building derived units from simple units and other derived units.
//       This header is just included as-is into units.h.
//       This is where the real meta-programming happens.  Unfortunately, it has to
//       be handled by the compiler at the same time as quantity<>.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UNITS_DERIVEDUNITS_H
#define UNITS_DERIVEDUNITS_H

namespace units
{
  //Derived unit support: I just need a special tag!
  template <class NUM, class DENOM> //Of course, NUM or DENOM could itself be a ratioTag<>
  class ratioTag
  {
    using num = NUM;
    using denom = DENOM;
  };
  
  //TODO: productTag<V, U> should be the same as productTag<U, V>.  I'll fix this if I ever need it.
  //TODO: Group like units so that cm * MeV * cm becomes cm^2 * MeV
  template <class ...PRODS>
  class productTag
  {
  };
  
  //This struct puts another simple tag into a productTag
  //Thank you Stack Overflow: https://stackoverflow.com/questions/9831501/how-can-i-have-multiple-parameter-packs-in-a-variadic-template
  template <class LHS, class RHS> 
  struct buildProduct
  {
    using result = productTag<LHS, RHS>;
  };
  
  //Specialization for productTag<> as both template parameters
  template <class ...LHS, class ...RHS>
  struct buildProduct<productTag<LHS...>, productTag<RHS...>>
  {
    using result = productTag<LHS..., RHS...>;
  };
  
  template <class ...LHS_NUM, class ...LHS_DENOM, class ...RHS>
  struct buildProduct<ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM...>>, productTag<RHS...>>
  {
    using result = ratioTag<productTag<LHS_NUM..., RHS...>, productTag<LHS_DENOM...>>;
  };
  
  template <class ...LHS, class ...RHS_NUM, class ...RHS_DENOM>
  struct buildProduct<productTag<LHS...>, ratioTag<productTag<RHS_NUM...>, productTag<RHS_DENOM...>>>
  {
    using result = ratioTag<productTag<LHS..., RHS_NUM...>, productTag<RHS_DENOM...>>;
  };
  
  template <class ...LHS_NUM, class ...LHS_DENOM, class ...RHS_NUM, class ...RHS_DENOM>
  struct buildProduct<ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM...>>, ratioTag<productTag<RHS_NUM...>, productTag<RHS_DENOM...>>>
  {
    using result = ratioTag<productTag<LHS_NUM..., RHS_NUM...>, productTag<LHS_DENOM..., RHS_DENOM...>>;
  };
  
  template <class LHS, class ...RHS>
  struct buildProduct<LHS, productTag<RHS...>>
  {
    using result = productTag<LHS, RHS...>;
  };
  
  template <class ...LHS, class RHS>
  struct buildProduct<productTag<LHS...>, RHS>
  {
    using result = productTag<LHS..., RHS>;
  };
  
  template <class LHS, class ...RHS_NUM, class ...RHS_DENOM>
  struct buildProduct<LHS, ratioTag<productTag<RHS_NUM...>, productTag<RHS_DENOM...>>>
  {
    using result = ratioTag<productTag<LHS, RHS_NUM...>, productTag<RHS_DENOM...>>;
  };
  
  template <class ...LHS_NUM, class ...LHS_DENOM, class RHS>
  struct buildProduct<ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM...>>, RHS>
  {
    using result = ratioTag<productTag<LHS_NUM..., RHS>, productTag<LHS_DENOM...>>;
  };
  
  //Construct a ratioTag<productTag<>, productTag<>> from ratioTag<>s and/or productTag<>s
  template <class LHS, class RHS>
  struct buildRatio
  {
    using result = ratioTag<productTag<LHS>, productTag<RHS>>;
  };
  
  //Specializations for ratios of productTag<> and ratioTag<>
  template <class ...LHS, class ...RHS>
  struct buildRatio<productTag<LHS...>, productTag<RHS...>>
  {
    using result = ratioTag<productTag<LHS...>, productTag<RHS...>>;
  };
  
  template <class ...LHS, class ...RHS_NUM, class ...RHS_DENOM>
  struct buildRatio<productTag<LHS...>, ratioTag<productTag<RHS_NUM...>, productTag<RHS_DENOM...>>>
  {
    using result = ratioTag<productTag<LHS..., RHS_DENOM...>, productTag<RHS_NUM...>>;
  };
  
  template <class ...LHS_NUM, class ...LHS_DENOM, class ...RHS>
  struct buildRatio<ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM...>>, productTag<RHS...>>
  {
    using result = ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM..., RHS...>>;
  };
  
  template <class ...LHS_NUM, class ...LHS_DENOM, class ...RHS_NUM, class ...RHS_DENOM>
  struct buildRatio<ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM...>>, ratioTag<productTag<RHS_NUM...>, productTag<RHS_DENOM...>>>
  {
    using result = ratioTag<productTag<LHS_NUM..., RHS_DENOM...>, productTag<LHS_DENOM..., RHS_NUM...>>;
  };
  
  //Specializations for feeding in just a single parameter and a productTag<> or a ratioTag<>
  template <class LHS, class ...RHS>
  struct buildRatio<LHS, productTag<RHS...>>
  {
    using result = ratioTag<productTag<LHS>, productTag<RHS...>>;
  };
  
  template <class ...LHS, class RHS>
  struct buildRatio<productTag<LHS...>, RHS>
  {
    using result = ratioTag<productTag<LHS...>, productTag<RHS>>;
  };
  
  template <class LHS, class ...RHS_NUM, class ...RHS_DENOM>
  struct buildRatio<LHS, ratioTag<productTag<RHS_NUM...>, productTag<RHS_DENOM...>>>
  {
    using result = ratioTag<typename buildProduct<LHS, RHS_DENOM...>::result, productTag<RHS_NUM...>>;
  };
  
  template <class ...LHS_NUM, class ...LHS_DENOM, class RHS>
  struct buildRatio<ratioTag<productTag<LHS_NUM...>, productTag<LHS_DENOM...>>, RHS>
  {
    using result = ratioTag<productTag<LHS_NUM...>, typename buildProduct<LHS_DENOM..., RHS>::result>;
  };
}

#endif //UNITS_DERIVEDUNITS_H
