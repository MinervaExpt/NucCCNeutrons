//File: BranchPtr.cpp
//Brief: A BranchPtr is a pointer to data obtained from a TBranch.  Data is only read from the TTree if
//       a BranchPtr is derefenced.  Data is ready every time it is dereferenced, so consider caching
//       the result.  BranchPtr works with std::vector<> and std::array<> as well as scalar types.  A BranchPtr is only valid as long as the tree it was created from exists.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef APO_BRANCHPTR_CPP
#define APO_BRANCHPTR_CPP

//ROOT includes
#include "TLeaf.h"
#include "TTree.h"

//c++ includes
#include <string>
#include <memory>
#include <cassert>
#ifndef NDEBUG
  #include <iostream>
#endif

namespace apo
{
  //I'm going to create an exception class below for each instance of BranchPtr<>.
  //I want to be able to catch any kind of BranchPtr<>::exception without checking for each type BranchPtr<> might be instantiated with.
  //So, I'm creating a base class they'll all derive from.  I can catch
  //this base class and still have the option to handle BranchPtr<>::exceptions individually too.
  class FailedToLoadBranch: public std::runtime_error
  {
    public:
      FailedToLoadBranch(const std::string& what): std::runtime_error(what.c_str())
      {
      }

      virtual ~FailedToLoadBranch() = default;
  };

  template <class T>
  class BranchPtr
  {
    public:
      class exception: public FailedToLoadBranch
      {
        public:
          exception(const std::string& what): FailedToLoadBranch(what)
          {
          }

          virtual ~exception() = default;
      };

      BranchPtr(const std::string& name, TTree& parent)
      {
        auto branch = parent.GetBranch(name.c_str());
        if(branch == nullptr) branch = parent.FindBranch(name.c_str());
        if(branch == nullptr) throw exception("Failed to get branch named "+name+"\n");

        parent.SetBranchStatus(name.c_str(), 1);
        fImplementation.reset(new Impl<T>(branch->GetLeaf(name.c_str())));
      }

      T operator *()
      {
        return fImplementation->Get();
      }

    private:
      //I only need to specialize some of th behavior of this class.
      //So, create an Impl class that I can specialize per-type to
      //retrieve specific types differently (i.e. std::vector<>) and
      //do some type-specific error checking.  This centralizes a lot
      //of the machinery in BranchPtr.
      template <class U>
      class Impl
      {
        public:
          Impl(TLeaf* leaf): fLeaf(leaf)
          {
            #ifndef NDEBUG
              std::cout << "Reading branch " << fLeaf->GetName() << " as a scalar.\n";
            #endif

            assert(fLeaf != nullptr && "You tried to create a BranchPtr<> with a nullptr TLeaf*!  I have no idea how you did this.");
            assert(fLeaf->GetLenType() * fLeaf->GetLen() == sizeof(T)
               && "TTree data format and sizeof(type you're reading) don't match!  Check TTree::Print()!");
          }

          U Get()
          {
            return *static_cast<U*>(fLeaf->GetValuePointer());
          }

        private:
          TLeaf* fLeaf; //Observer pointer to leaf from which data will be retrieved.
      };

      //Specialize for std::vector<>
      template <class U>
      class Impl<std::vector<U>>
      {
        public:
          Impl(TLeaf* leaf): fLeaf(leaf)
          {
            #ifndef NDEBUG
              std::cout << "Reading branch " << fLeaf->GetName() << " as a vector<>.\n";
            #endif

            assert(fLeaf != nullptr && "You tried to create a BranchPtr<> with a nullptr TLeaf*!  I have no idea how you did this.");
            assert(fLeaf->GetLenType() == sizeof(U)
               && "TTree data format and sizeof(type in the vector<> you're reading) don't match!  Check TTree::Print()!");
          }

          std::vector<U> Get()
          {
            auto begin = static_cast<U*>(fLeaf->GetValuePointer());
            return std::vector<U>(begin, begin + fLeaf->GetLen());
          }

        private:
          TLeaf* fLeaf; //Observer pointer to leaf from which data will be retrieved.
      };

      //Specialize for std::array<>
      template <class U, int N>
      class Impl<std::array<U, N>>
      {
        public:
          Impl(TLeaf* leaf): fLeaf(leaf)
          {
            #ifndef NDEBUG 
              std::cout << "Reading branch " << fLeaf->GetName() << " as a vector<>.\n";
            #endif
            
            assert(fLeaf != nullptr && "You tried to create a BranchPtr<> with a nullptr TLeaf*!  I have no idea how you did this.");
            assert(fLeaf->GetLenType() * fLeaf->GetLen() == sizeof(std::array<U, N>)
               && "TTree data format and sizeof(array<> you're reading) don't match!  Check TTree::Print()!");
          }

          std::array<U, N> Get()
          {
            assert(fLeaf->GetLen() == N && "TTree data length for this entry doesn't match number of entries in array<> you're trying to read!  If you're trying"
                  "to read a \"variable-sized array\", use std::vector<> instead.");
            auto begin = static_cast<T*>(fLeaf->GetValuePointer());
            return std::array<T, N>(begin, begin + N);
          }

        private:
          TLeaf* fLeaf; //Observer pointer to leaf from which data will be retrieved.
      };

      std::unique_ptr<Impl<T>> fImplementation; //Hold a unique_ptr<> so I
                                                //can delay initialization
  };
}
#endif //APO_BRANCHPTR_CPP
