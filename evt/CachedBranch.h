//File: CachedBranch.h
//Brief: Read-on-first-use storage for a branch value.
//Author: Andrew Olivier aolivier@ur.rochester.edu

namespace evt
{
  template <(Universe::*getBranch)()>
  class CachedBranch
  {
    private:
      using TYPE = decltype((std::declval<Universe>().*getBranch)());

    public:
      CachedBranch()
      {
        reset();
      }

      TYPE operator()(const Universe& evt)
      {
        return (this->*fStrategy)(evt);
      }

      //Call on each new AnaTuple entry
      //TODO: Just let the cache go out of scope at entry loop level?  Seems like a lot of memory allocations
      void reset()
      {
        fStrategy = &CachedBranch::getFromAnaTuple;
      }

    private:
      TYPE fValue; //cached value from a branch

      (CachedBranch::*fStrategy)(const Universe&); //Current strategy for retrieving the number in fValue

      //Options for fStrategy are these member functions:
      TYPE getFromAnaTuple(const Universe& evt)
      {
        fValue = (evt.*getBranch)(); //Call this before changing fStrategy for exception saftey.  If getBranch() throws an exception that is caught, the next
                                     //call to operator() will try to fill fValue again instead of reading a default-initialized value.
        fStrategy = &CachedBranch::getFromCache; //For next time
        return fValue;
      }

      TYPE getFromCache(const Universe& /*evt*/)
      {
        return fValue;
      }
  };
}
