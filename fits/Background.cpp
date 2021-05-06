//File: Background.cpp
//Brief: A Background scale factor calculation for sideband fits.  Derive from this class and implement functionToFit()
//       and other pure virtuals to use it.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//fit includes
#include "Background.h"
#include "Sideband.h"

//ROOT includes
#include "Math/Minimizer.h"

//c++ includes
#include <cassert>
#include <algorithm>

namespace fit
{
  std::unique_ptr<TH1D> Background::makeDataMCRatio(const Sideband& sideband, const double POTRatio) const
  {
    std::unique_ptr<TH1D> mcRatio(static_cast<TH1D*>(sideband.fixedSum->Clone()));
    for(const auto& hist: sideband.floatingHists) mcRatio->Add(hist);
    mcRatio->Scale(POTRatio);
    mcRatio->Divide(&sideband.data, mcRatio.get());
  
    return std::move(mcRatio);
  }
  
  auto Background::getMostPureSideband(const std::vector<Sideband>& sidebands) const -> decltype(sidebands.begin())
  {
    assert(!sidebands.empty());
  
    const auto whichHist = std::find_if(sidebands.front().floatingHists.begin(), sidebands.front().floatingHists.end(), [this](const auto hist) { return std::string(hist->GetName()).find(this->name) != std::string::npos; });
    assert(whichHist != sidebands.front().floatingHists.end());
    const size_t index = std::distance(sidebands.front().floatingHists.begin(), whichHist);
  
    const auto largestSideband = std::max_element(sidebands.begin(), sidebands.end(),
                                                  [index](const auto& lhs, const auto& rhs)
                                                  {
                                                    const auto sumHists = [](const double sum, const auto hist) { return sum + hist->Integral(); };
                                                    const double lhsTotal = std::accumulate(lhs.floatingHists.begin(), lhs.floatingHists.end(), lhs.fixedSum->Integral(), sumHists);
                                                    const double rhsTotal = std::accumulate(rhs.floatingHists.begin(), rhs.floatingHists.end(), rhs.fixedSum->Integral(), sumHists);
                                                    return lhs.floatingHists[index]->Integral()/lhsTotal < rhs.floatingHists[index]->Integral()/rhsTotal;
                                                  });
  
    return largestSideband;
  }
}
