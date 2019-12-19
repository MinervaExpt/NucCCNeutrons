//File: Binned.cpp
//Brief: A Binned<HIST> bins HISTs in some variable.  So, filling
//       the bin for a value q3 looks like:
//
//       ::Binned<TH1F> q3ToNCandidates(bins, "nCandidates", "LE ", varName, nSigFigs, " GeV/c;Candidates;Events", outSentry, 4, 0, 4);
//       //... get access to interesting data in a variable named q3 ...
//       q3ToNCandidates[q3].Fill(nCand);
//
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_BINNED_CPP
#define APO_BINNED_CPP 

//Local includes
#include "DirSentry.cpp"
#include "stringify.cpp"
#include "SafeROOTName.h"

//ROOT includes
#include "TDirectory.h"

//c++ includes
#include <string>
#include <initializer_list>
#include <map>

namespace apo
{
  //Manages a group of TObject-derived HISTs with one HIST per bin.  Bins are specified by lower edges of type
  //FLT_PT_TYPE = float_t.  FLT_PT_TYPE must be comparable with std::less()
  template <class HIST, class FLT_PT_TYPE = float>
  class Binned
  {
    public:
      using float_t = FLT_PT_TYPE;

      //Create HISTs in parent
      template <class ...BINARGS>
      Binned(const std::initializer_list<const float_t> bins, const std::string& name, const std::string& title,
             const std::string& var, const size_t nSigFigs, const std::string& axes, TDirectory& parent, BINARGS... args)
      {
        apo::DirSentry sentry(parent);
        createHists(bins, name, title, var, nSigFigs, axes, args...);
      }

      //Allow Binned<> to be composable
      template <class ...BINARGS>
      Binned(const std::string& name, const std::string& title, const std::initializer_list<const float_t> bins,
             const std::string& var, const size_t nSigFigs, const std::string& axes, TDirectory& parent, BINARGS... args)
      {
        apo::DirSentry sentry(parent); 
        createHists(bins, name, title, var, nSigFigs, axes, args...);
      }

      //Create HISTs in the current TDirectory.
      //It doesn't make sense to construct a binned without a TDirectory at all because
      //it would leak its memory.  Changing those pointers to just HISTs would help,
      //but then I'd have to write template magic to "figure out" whether this Binned<>
      //will be used with a TFile or not.
      template <class ...BINARGS>
      Binned(const std::initializer_list<const float_t> bins, const std::string& name, const std::string& title,
             const std::string& var, const size_t nSigFigs, const std::string& axes, apo::DirSentry& /*sentry*/, BINARGS... args)
      {
        createHists(bins, name, title, var, nSigFigs, axes, args...);
      }

      HIST& operator[](const float_t value)
      {
        const auto found = fVarToHist.upper_bound(value);
        if(found != fVarToHist.begin()) return *(std::prev(found)->second);
        return *fUnderflow;
      }

      template <class ...ARGS>
      auto Fill(const float_t value, ARGS... args)
      {
        (*this)[value]->Fill(args...);
      }

      //Apply a callable object, of type FUNC, to each histogram this object manages.
      //FUNC takes the bin lower edge and a reference to the histogram as argument.
      template <class FUNC>
      void visit(FUNC&& func)
      {
        for(auto& bin: fVarToHist) func(bin.first, *(bin.second));
        func(fVarToHist.begin()->first - 1., *fUnderflow);
      }

    private:
      //Every HIST is stored as a HIST* because they are really owned by the current TDirectory.
      //So, these are all observer pointers that will not be deleted.
      std::map<float_t, HIST*> fVarToHist; //Mapping from variable at lower bin edge to histogram
      HIST* fUnderflow; //Underflow bin

      //Helper function to centralize constructor functionality
      template <class ...BINARGS>
      void createHists(const std::initializer_list<const float_t> bins, const std::string& name, const std::string& title,
                       const std::string& var, const size_t nSigFigs, const std::string& axes, BINARGS... args)
      {
        std::string binStr = apo::stringify(*(bins.begin()), nSigFigs);

        //Underflow bin for values below first bin's lower edge
        fUnderflow = new HIST((name + "underflow ").c_str(), (title + var + " < " + binStr + axes).c_str(), args...);

        //Middle bin values
        for(auto bin = bins.begin(); bin < std::prev(bins.end()); ++bin)
        {
          const auto nextBin = apo::stringify(*std::next(bin), nSigFigs);
          fVarToHist[*bin] = new HIST((name + SafeROOTName(binStr + " ")).c_str(),
                                      (title + binStr + " < " + var + " < " + nextBin + axes).c_str(), args...);
          binStr = nextBin;
        }

        //Last bin has overflow values
        binStr = apo::stringify(*std::prev(bins.end()), nSigFigs);
        fVarToHist[*std::prev(bins.end())] = new HIST((name + SafeROOTName(binStr + " ")).c_str(),
                                                      (title + var + " > " + binStr + axes).c_str(), args...);
      }
  };
}

#endif //APO_BINNED_CPP
