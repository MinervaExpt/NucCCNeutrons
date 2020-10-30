//File: Directory.h
//Brief: Directory wraps over any class that implements a contract like 
//       art::TFileDirectory to expose the same contract without creating 
//       any sub-TDirectoires.  Instead, Directory::make<>() appends its 
//       name to objects' names to give some strong assurances of name 
//       uniqueness.  Directory might be useful in adapting code that 
//       normally relies on TFileDirectory-like objects to work with 
//       post-processing software like Monet that can't deal with nested 
//       TDirectories. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UTIL_DIRECTORY_H
#define UTIL_DIRECTORY_H

//c++ includes
#include <string>

//ROOT includes
#include "TFile.h"
#include "TH1.h"

//PlotUtils includes
//No junk from PlotUtils please!  I already
//know that MnvH1D does horrible horrible things.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#pragma GCC diagnostic pop

//Units includes
#include "util/WithUnits.h"

namespace util  
{
  //Machinery to specialize just one bit of code
  //that sets the directory of a created object.
  //If that object happens to be a HistWrapper or
  //a Hist2DWrapper, I want to look up its underlying
  //histogram.
  namespace detail
  {
    template <class TOBJ>
    struct set
    {
      static void dir(TOBJ& /*obj*/, TDirectory& /*dir*/) {}
      static void dir(TH1& hist, TDirectory& dir) { hist.SetDirectory(&dir); }
    };

    //Specialization for HistWrapper<>
    template <class UNIV>
    struct set<PlotUtils::HistWrapper<UNIV>>
    {
      static void dir(PlotUtils::HistWrapper<UNIV>& wrapper, TDirectory& dir) { wrapper.hist->SetDirectory(&dir); }
    };

    //Specialization for Hist2DWrapper<>
    template <class UNIV>
    struct set<PlotUtils::Hist2DWrapper<UNIV>>
    { 
      static void dir(PlotUtils::Hist2DWrapper<UNIV>& wrapper, TDirectory& dir) { wrapper.hist->SetDirectory(&dir); }
    };

    //Specialize for using HistWrapper<> with WithUnits<>.
    //I could probably do SFINAE and look for a "hist" member,
    //but it's after 6PM.  Actually, this might turn out to be
    //SFINAE after all...
    template <class HASHIST, class ...UNITS>
    struct set<units::WithUnits<HASHIST, UNITS...>>
    {
      static void dir(units::WithUnits<HASHIST, UNITS...>& wrapper, TDirectory& dir) { wrapper.hist->SetDirectory(&dir); }
    };
  }

  //Looks like art::TFileService.
  class Directory 
  {
    public:
      //Create a top-level Directory to wrap over some DIRECTORY 
      //object.  A Directory created this way will not append 
      //anything to the names of its children, but its sub-directories 
      //will.  
      Directory(TFile& dir);
      virtual ~Directory() = default;

      //TODO: I could make this behavior virtual, thus letting me choose at runtime
      //      whether to use nested TDirectories, by abstracting it into a virtual
      //      cd() function and a virtual prefix() function.
      //Implementation of TFileDirectory-like contract.  
      template <class TOBJECT, class ...ARGS>
      TOBJECT* make(const std::string& name, /*const std::string& title,*/ ARGS... args)
      {
        //TODO: Figure out which parameter(s) will set obj's name 
        //      without actually creating an object with that name.  
        //      This would avoid some temporary name conflicts that 
        //      would occur with a "real" TDirectory.  
        sentry dirSentry;
        fBaseDir.cd();
        auto obj = new TOBJECT((fName + name).c_str(), /*title.c_str(),*/ args...);
        detail::set<TOBJECT>::dir(*obj, fBaseDir); //This is redundant for normal TH1Ds, but it's necessary for
                                                   //PlotUtils::HistWrapper<> because HistWrapper<> calls SetDirectory(0)
                                                   //in its constructor.
        return obj;
      }

      Directory mkdir(const std::string& name);

      //Move a TH1 into this Directory.
      //Only TH1-derived classes have SetDirectory().
      void mv(TH1* obj);
       
    private:
      TFile& fBaseDir; //Base directory in which this 
                       //Directory and all of its 
                       //children will put objects they 
                       //make<>().  

      const std::string fName; //The name of this Directory.  Will be 
                               //appended to the names of all child objects
                               //to create unique names.  

      //Create a subdirectory of a given Directory.  This behavior is 
      //exposed to the user through mkdir.
      Directory(const std::string& name, Directory& parent);

      static constexpr auto Separator = "_"; //The separator between nested FlatDirectories' names.  
                                             //I'll probably never change it, but I've written a 
                                             //parameter here so that any future changes can be 
                                             //maintained in one place.  
                                             //
                                             //Worrying so much about the storage "policy" for 
                                             //what is probably just a C-string is serious overkill.  

      struct sentry
      {
        sentry(): fOldPwd(gDirectory) {}
        ~sentry() { fOldPwd->cd(); }

        private:
          TDirectory* fOldPwd;
      };
  };
}

#endif //UTIL_DIRECTORY_H  
