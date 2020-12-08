//File: TruthInteractionCategories.h
//Brief: Useful CATEGORYs for analyses that go with Categorized.
//       TODO: Convert them into truth::Cuts?
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef ANA_TRUTHINTERACTIONCATEGORIES_H
#define ANA_TRUTHINTERACTIONCATEGORIES_H

//c++ includes
#include <unordered_set>

namespace ana
{
  //Categorize by the interaction GENIE generated.
  //Based on physics mechanisms BEFORE FSI.
  //GENIEEnum is chosen to match MINERvA's offline Gaudi-based
  //framework.  As far as I know, this means GENIE 2.12.6 as of
  //the time of writing.
  const std::map<int, std::string> GENIECategories = {{1, "QE"},
                                                      {8, "2p2h"},
                                                      {2, "RES"},
                                                      {3, "DIS"}
                                                      //Other is built in for free
                                                     };
  using GENIECategory = int;

  //Categorize by truth particles in the Final State.
  //More interesting for comparison to reconstruction
  //because these are computed AFTER the Final State
  //Interaction (FSI) simulation.
  class FSCategory
  {
    public:
      FSCategory(const std::string& name, const std::unordered_set<int> forbidden, const std::unordered_set<int> required): fName(name), fForbidden(forbidden), fRequired(required)
      {
      }

      FSCategory(const std::string& name, const std::unordered_set<int> forbidden): fName(name), fForbidden(forbidden)
      {
      }

      const std::string& name() const { return fName; }

      bool operator ()(const evt::Universe& univ) const
      {
        for(const int pdg: univ.GetFSPDGCodes())
        {
          //TODO: I'm not distinguishing between particles and anti-particles
          if(fForbidden.count(fabs(pdg))) return false;
        }

        return true;
      }

    private:
      const std::string fName; //Name to satisfy util::Categorized<>
      const std::unordered_set<int> fForbidden; //PDG codes forbidden in this category definition
      const std::unordered_set<int> fRequired; //PDG codes required in this category definition
  };

  const std::vector<FSCategory*> pionFSCategories = { new FSCategory("QE-like", {211, 111, 321, 311}),
                                                      new FSCategory("Charged Pions", {111, 321, 311}, {211}),
                                                      new FSCategory("Neutral Pions", {321, 311}, {111})
                                                    };
}

#endif //ANA_TRUTHINTERACTIONCATEGORIES_H
