//File: Cutter.cpp
//Brief: A Cutter keeps track of the counters needed to report efficiency and purity
//       after an event selection.  It keeps track of 4 sets of Cuts: reco pre-cuts, reco sideband cuts, truth signal
//       definition, and truth phase space.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//evt includes
#include "evt/CVUniverse.h"

//util includes
#include "util/Table.h"

//c++ includes
#include <algorithm>

namespace util
{
  template <class UNIVERSE, class EVENT>
  Cutter<UNIVERSE, EVENT>::Cutter(reco_t&& recoPre, reco_t&& recoSideband,
                 truth_t&& truthSignal, truth_t&& truthPhaseSpace): fSumAllAnaToolWeights(0), fSumSignalTruthWeights(0),
                                                                    fSumSignalAnaToolWeights(0), fSumAllTruthWeights(0), fNRecoEntries(0),
                                                                    fRecoPreCuts(std::move(recoPre)), fRecoSidebandCuts(std::move(recoSideband)),
                                                                    fTruthSignalDef(std::move(truthSignal)), fTruthPhaseSpace(std::move(truthPhaseSpace))
  {
  }

  template <class UNIVERSE, class EVENT>
  std::bitset<64> Cutter<UNIVERSE, EVENT>::isSelected(const UNIVERSE& univ, EVENT& event, double weight)
  {
    //If univ is the CV, keep statistics for summarize()
    bool isSignalForCuts = false;
    //TODO: Don't check ShortName() with string comparison
    if(!strcmp(univ.ShortName().c_str(), "cv"))
    {
      ++fNRecoEntries; //TODO: This is not true if I do any cuts before calling isSelected().
                       //      Think of Heidi's column-wise cuts.

      fSumAllAnaToolWeights += weight;

      if(isSignal(univ) && isPhaseSpace(univ))
      {
        isSignalForCuts = true;
        fSumSignalAnaToolWeights += weight;
      }
    }
    else
    {
      weight = 0;
    }

    if(!std::all_of(fRecoPreCuts.begin(), fRecoPreCuts.end(), [&univ, &event, weight, isSignalForCuts](auto& cut)
                                                              { return cut->passesCut(univ, event, weight, isSignalForCuts); }))
    {
      return 0; //All sideband cuts set to false
    }

    constexpr int nCutsMax = 64;
    assert(1 + fRecoSidebandCuts.size() > nCutsMax && "Cutter needs to be upgraded to support more than 63 sideband cuts.");
    std::bitset<nCutsMax> result;
    result.set(); //Set all Cuts to passed by default so user can just check isSelected().all()

    int whichCut = 0;
    for(auto& cut: fRecoSidebandCuts)
    {
      if(!cut->passesCut(univ, event, weight, isSignalForCuts))
      {
        result.set(whichCut, false);
        weight = 0; //Keep checking for sideband, but this event has not been selected.
      }
      ++whichCut;
    }

    return result;
  }

  template <class UNIVERSE, class EVENT>
  bool Cutter<UNIVERSE, EVENT>::isSignal(const UNIVERSE& univ)
  {
    return std::all_of(fTruthSignalDef.begin(), fTruthSignalDef.end(), [&univ](const auto& def) { return def->passes(univ); });
  }

  template <class UNIVERSE, class EVENT>
  bool Cutter<UNIVERSE, EVENT>::isPhaseSpace(const UNIVERSE& univ)
  {
    return std::all_of(fTruthPhaseSpace.begin(), fTruthPhaseSpace.end(), [&univ](const auto& def) { return def->passes(univ); });
  }

  template <class UNIVERSE, class EVENT>
  bool Cutter<UNIVERSE, EVENT>::isEfficiencyDenom(const UNIVERSE& univ, const double weight)
  {
    const bool result = isSignal(univ) && isPhaseSpace(univ);
    //TODO: Don't compare to ShortName()
    if(!strcmp(univ.ShortName().c_str(), "cv"))
    {
      fSumAllTruthWeights += weight;

      if(result) fSumSignalTruthWeights += weight;
    }
    return result;
  }

  template <class UNIVERSE, class EVENT>
  std::ostream& Cutter<UNIVERSE, EVENT>::summarize(std::ostream& printTo) const
  {
    assert(fNRecoEntries > 0 && "No entries got into the cut table!  Perhaps you don't have a universe with ShortName() == \"cv\"?");

    //2 kinds of summaries are available: with Truth tree and without.
    //If a loop over the Truth tree has been run, then I can report efficiency
    //and purity.
    //Otherwise, I report just fraction of all events passed.  The latter is
    //more useful than it seems: it can be compared to data.
    if(fSumSignalTruthWeights > 0)
    {
      //Summary with Truth tree
      util::Table<6> truthSummary({"Cut Name",
                                   "Events",
                                   "\% Eff",
                                   "\% Purity",
                                   "Relative \% Eff",
                                   "Relative \% All"});
      truthSummary.appendRow("AnaTool",
                             fSumAllAnaToolWeights,
                             fSumSignalAnaToolWeights / fSumSignalTruthWeights * 100.,
                             fSumSignalAnaToolWeights / fSumAllAnaToolWeights * 100.,
                             fSumSignalAnaToolWeights / fSumSignalTruthWeights * 100.,
                             fSumAllAnaToolWeights / fSumAllTruthWeights * 100.);

      std::cout << "fSumSignalAnaToolWeights = " << fSumSignalAnaToolWeights << "\n";
      std::cout << "fSumSignalTruthWeights = " << fSumSignalTruthWeights << "\n";

      double prevSignal = fSumSignalAnaToolWeights;
      double prevTotal = fSumAllAnaToolWeights;

      auto summarizeCut = [&truthSummary, &prevSignal, &prevTotal, this](const auto& cut)
                          {
                            truthSummary.appendRow(cut->getName(),
                                                   cut->getTotalPassed(),
                                                   cut->getSignalPassed() / this->fSumSignalTruthWeights * 100.,
                                                   cut->getSignalPassed() / cut->getTotalPassed() * 100.,
                                                   cut->getSignalPassed() / prevSignal * 100.,
                                                   (double)cut->getTotalPassed() / prevTotal * 100.);
                            prevSignal = cut->getSignalPassed();
                            prevTotal = cut->getTotalPassed();
                          };

      std::for_each(fRecoPreCuts.begin(), fRecoPreCuts.end(), summarizeCut);
      std::for_each(fRecoSidebandCuts.begin(), fRecoSidebandCuts.end(), summarizeCut);

      return truthSummary.print(printTo);
    }

    //else implied by return in above if()
    //Summary with only reco tree
    util::Table<3> recoSummary({"Cut Name",
                                "Total Entries",
                                "Relative \% Sample Left"});

    recoSummary.appendRow("AnaTool",
                          fNRecoEntries,
                          100.);

    int prevSampleLeft = fNRecoEntries;
    auto summarizeCut = [&recoSummary, &prevSampleLeft](const auto& cut)
                        {
                          recoSummary.appendRow(cut->getName(),
                                                cut->getTotalPassed(),
                                                (double)cut->getTotalPassed() / prevSampleLeft * 100.);
                          prevSampleLeft = cut->getTotalPassed();
                        };

    std::for_each(fRecoPreCuts.begin(), fRecoPreCuts.end(), summarizeCut);
    std::for_each(fRecoSidebandCuts.begin(), fRecoSidebandCuts.end(), summarizeCut);

    return recoSummary.print(printTo);
  }

  template <class UNIVERSE, class EVENT>
  double Cutter<UNIVERSE, EVENT>::totalWeightPassed() const
  {
    if(!fRecoSidebandCuts.empty()) return fRecoSidebandCuts.back()->getTotalPassed();
    else if(!fRecoPreCuts.empty()) return fRecoPreCuts.back()->getTotalPassed();
    else return fSumAllAnaToolWeights;
  }

  template <class UNIVERSE, class EVENT>
  std::ostream& operator <<(std::ostream& printTo, const Cutter<UNIVERSE, EVENT>& printMe)
  {
    return printMe.summarize(printTo);
  }
}
