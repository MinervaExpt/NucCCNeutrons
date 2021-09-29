//File: Categorized.h
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
#include "util/Directory.h"

//c++ includes
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <functional>
#include <type_traits>

namespace
{
  //Check whether a CATegory is std::hash<>able.  Soft of a missing piece of the STL because
  //it wasn't guaranteed to work pre-c++17.  As long as you're using a c++17-era STL, this should work
  //regardless of compiler standard.
  //Solution inspired by https://stackoverflow.com/questions/12753997/check-if-type-is-hashable

  template <class CAT, class VALUE, typename = std::void_t<>>
  struct map_t { using type = std::map<CAT, VALUE>; };

  template <class CAT, class VALUE>
  struct map_t<CAT, VALUE, std::void_t<decltype(std::declval<std::hash<CAT>>()(std::declval<CAT>()))>>
  {
    using type = std::unordered_map<CAT, VALUE>;
  };
}

namespace util
{
  //Mapping from a set of values to a name.  Helper for constructing a Categorized<>
  template <class value_t>
  struct NamedCategory
  {
    std::vector<value_t> values;
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
    private:
      //Code reuse for the special case of std::unique_ptr<> through metaprogramming.
      //Solves the problem where I want to hash std::unique_ptr<>s without owning them.

      //The "normal" case: It's fine to keep a copy of CAT.
      template <class CAT>
      struct lookup
      {
        typename map_t<CAT, HIST*>::type map;

        HIST*& operator [](const CAT& key) { return map[key]; }

        auto find(const CAT& key) const -> decltype(map.find(key)) { return map.find(key); }
      };

      //Special case: Don't hold on to a copy of unique_ptr<>.  Take an
      //              observer pointer instead.
      template <class CAT>
      struct lookup<std::unique_ptr<CAT>>
      {
        typename map_t<CAT*, HIST*>::type map;
        static_assert(std::is_same<typename map_t<CAT*, HIST*>::type, std::unordered_map<CAT*, HIST*>>::value, "Using std::map for unique_ptr<> where unordered_map<> should be used!");

        HIST*& operator [](const std::unique_ptr<CAT>& key) { return map[key.get()]; }

        auto find(const std::unique_ptr<CAT>& key) const -> decltype(map.find(key.get())) { return map.find(key.get()); }
      };

    public:
      //CATEGORIES is an iterable container of objects like NamedCategory<>
      template <class ...HISTARGS>
      Categorized(const std::vector<NamedCategory<CATEGORY>>& categories, Directory& dir, const std::string& baseName,
                  const std::string& axes, HISTARGS... args)
      {
        for(const auto& category: categories)
        {
          auto hist = dir.make<HIST>(SafeROOTName(baseName + "_" + category.name).c_str(), (category.name + ";" + axes).c_str(), args...);
          for(const auto& value: category.values)
          {
            fCatToHist[value] = hist;
          }
        }

        fOther = dir.make<HIST>((baseName + "_Other").c_str(), ("Other;" + axes).c_str(), args...);
      }

      //CATEGORY is a pointer to an object with a name() member variable
      template <class ...HISTARGS>
      Categorized(const std::vector<CATEGORY>& categories, Directory& dir, const std::string& baseName,
                  const std::string& axes, HISTARGS... args)
      {
        for(const auto& catPtr: categories)
        {
          fCatToHist[catPtr] = dir.make<HIST>(SafeROOTName(baseName + "_" + catPtr->name()).c_str(), (catPtr->name() + ";" + axes).c_str(), args...);
        }

        fOther = dir.make<HIST>((baseName + "_Other"), ("Other;" + axes).c_str(), args...);
      }

      //CATEGORY is a pointer to an object with a name() member variable
      template <class ...HISTARGS>
      Categorized(const std::string& baseName, const std::string& axes,
                  const std::vector<CATEGORY>& categories, Directory& dir, HISTARGS... args)
      {
        for(const auto& catPtr: categories)
        {
          fCatToHist[catPtr] = dir.make<HIST>(SafeROOTName(baseName + "_" + catPtr->name()).c_str(), (catPtr->name() + ";" + axes).c_str(), args...);
        }

        fOther = dir.make<HIST>((baseName + "_Other"), ("Other;" + axes).c_str(), args...);
      }

      //Use a std::map<> instead of CATEGORIES
      template <class ...HISTARGS>
      Categorized(const std::string& baseName, const std::string& axes,
                  const std::map<CATEGORY, std::string> categories, Directory& dir, HISTARGS... args)
      {
        for(const auto& category: categories)
        {
          auto hist = dir.make<HIST>(SafeROOTName(baseName + "_" + category.second), (category.second + ";" + axes).c_str(), args...);
          fCatToHist[category.first] = hist;
        }

        fOther = dir.make<HIST>((baseName + "_Other").c_str(), ("Other;" + axes).c_str(), args...);
      }

      HIST& operator [](const CATEGORY& cat) const
      {
        //Find out whether category is kept track of separately
        const auto found = fCatToHist.find(cat);
        if(found == fCatToHist.map.end()) return *fOther; //If not, lump this entry in with other uncategorized entries
        return *found->second; //If so, return its category
        
      }

      //Apply a callable object, of type FUNC, to each histogram this object manages.
      //FUNC takes only a reference to the histogram as argument.
      template <class FUNC>
      void visit(FUNC&& func)
      {
        //Make sure that each histogram is normalized exactly once
        std::set<HIST*> histsNormalized;
        for(auto& category: fCatToHist.map)
        {
          if(histsNormalized.count(category.second) == 0)
          {
            func(*(category.second));
            histsNormalized.insert(category.second);
          }
        }

        func(*fOther);
      }

      void SetDirectory(TDirectory* dir)
      {
        visit([dir](auto& hist) { util::detail::set<HIST>::dir(hist, *dir); });
      }

    private:
      //All HISTs are referred to as observer pointers for compatability with TH1s created in a TFile.
      //The TFile is responsible for deleting them.
      lookup<CATEGORY> fCatToHist;
      HIST* fOther; //All entries that don't fit in any other CATEGORY end up in this HIST
  };
}

#endif //UTIL_CATEGORIZED_CPP
