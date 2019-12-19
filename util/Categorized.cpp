//File: Categorized.cpp
//Brief: HISTs can be Categorized<HIST> according to a NamedCategory<> to make
//       a separate HIST for each NamedCategory<> that can be Fill()ed based on
//       which NamedCategory<> a value belongs to.  Useful for putting together
//       stacked histograms that compare different "channels" with how they
//       contribute to the total histogram for a value.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UTIL_CATEGORIZED_CPP
#define UTIL_CATEGORIZED_CPP

//Local includes
#include "util/SafeROOTName.h"

//c++ includes
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

namespace util
{
  //Mapping from a set of values to a name.  Helper for constructing a Categorized<>
  template <class value_t>
  struct NamedCategory
  {
    std::vector<const value_t> values;
    std::string name;
  };

  //A Categorized holds a total HIST along with a HIST for each category.
  //It works similarly to a Binned<>, but each entry either exactly matches
  //one CATEGORY or is put in the Other CATEGORY.
  //Its Fill() method takes a CATEGORY plus whatever ARGS HIST::Fill() takes.
  template <class HIST, class CATEGORY>
  //HIST is a Fill()able type that takes a c-string as its first constructor argument (like TH1D or Binned<TH1D>)
  //CATEGORY is hashable for std::unordered_map<> and has either a "name" member that can be added to a std::string
  //         or first and second members with first being hashable and second that can be added to a std::string.
  class Categorized
  {
    public:
      //CATEGORIES is an iterable container of objects like NamedCategory<>
      template <class CATEGORIES, class ...HISTARGS>
      Categorized(const CATEGORIES categories, Directory& dir, HISTARGS... args)
      {
        for(const auto& category: categories)
        {
          auto hist = dir.make<HIST>(SafeROOTName(category.name).c_str(), category.name.c_str(), args...);
          for(const auto& value: category.values)
          {
            fCatToHist[value] = hist;
          }
        }

        fOther = dir.make<HIST>("Other", "Other", args...);
      }

      //Use a std::map<> instead of CATEGORIES
      template <class ...HISTARGS>
      Categorized(const std::map<CATEGORY, std::string> categories, Directory& dir, HISTARGS... args)
      {
        for(const auto& category: categories)
        {
          auto hist = dir.make<HIST>(SafeROOTName(category.second), category.second.c_str(), args...);
          fCatToHist[category.first] = hist;
        }

        fOther = dir.make<HIST>("Other", "Other", args...);
      }

      HIST& operator [](const CATEGORY& cat) const
      {
        //Find out whether category is kept track of separately
        const auto found = fCatToHist.find(category);
        if(found == fCatToHist.end()) return *fOther; //If not, lump this entry in with other uncategorized entries
        return *found->second; //If so, return its category
        
      }

      //Apply a callable object, of type FUNC, to each histogram this object manages.
      //FUNC takes only a reference to the histogram as argument.
      template <class FUNC>
      void visit(FUNC&& func)
      {
        //Make sure that each histogram is normalized exactly once
        std::set<HIST*> histsNormalized;
        for(auto& category: fCatToHist)
        {
          if(histsNormalized.count(category.second) == 0)
          {
            func(*(category.second));
            histsNormalized.insert(category.second);
          }
        }

        func(*fOther);
      }

    private:
      //All HISTs are referred to as observer pointers for compatability with TH1s created in a TFile.
      //The TFile is responsible for deleting them.
      std::unordered_map<CATEGORY, HIST*> fCatToHist; //Mapping from CATEGORY to HISTs in that category
      HIST* fOther; //All entries that don't fit in any other CATEGORY end up in this HIST
  };
}

#endif //UTIL_CATEGORIZED_CPP
