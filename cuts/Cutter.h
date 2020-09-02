//File: Cutter.h
//Brief: A Cutter keeps track of the counters needed to report efficiency and purity
//       after an event selection.  It keeps track of 4 sets of Cuts: reco pre-cuts, reco sideband cuts, truth signal
//       definition, and truth phase space.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cuts includes
#include "cuts/reco/Cut.h"
#include "cuts/truth/Cut.h"

//c++ includes
#include <iostream>
#include <memory>
#include <bitset>

namespace evt
{
  class CVUniverse;
}

namespace util
{
  class Cutter
  {
    public:
      using reco_t = std::vector<std::unique_ptr<reco::Cut>>;
      using truth_t = std::vector<std::unique_ptr<truth::Cut>>;

      Cutter(reco_t&& recoPre, reco_t&& recoSideband, truth_t&& truthSignal, truth_t&& truthPhaseSpace);

      //Hooks to keep statistics within your event loop.

      //Look for isSelected().all() for the main analysis.
      //isSelected()[n] is the status of the nth sideband cut.
      //isSelected().none() means the pre-cuts failed and no sidebands were checked to save computation time.
      std::bitset<64> isSelected(const evt::CVUniverse& univ, const double weight);
      //isSignal() && isPhaseSpace() with some extra bookkeeping to report overall efficiency and purity.
      bool isEfficiencyDenom(const evt::CVUniverse& univ, const double weight);

      //No statistics kept for these
      bool isSignal(const evt::CVUniverse& univ);
      bool isPhaseSpace(const evt::CVUniverse& univ);

      std::ostream& summarize(std::ostream& printTo) const;
      double totalWeightPassed() const; //Get total weight that passed all cuts

    private:
      double fSumAllAnaToolWeights;
      double fSumSignalTruthWeights;
      double fSumSignalAnaToolWeights;
      double fSumAllTruthWeights;
      long int fNRecoEntries;

      reco_t fRecoPreCuts;
      reco_t fRecoSidebandCuts;
      truth_t fTruthSignalDef;
      truth_t fTruthPhaseSpace;
  };

  std::ostream& operator <<(std::ostream& printTo, const Cutter& printMe);
}
