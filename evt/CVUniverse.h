//File: CVUniverse.h
//Brief: Adjusts BaseUniverse information to represent a Central Value Universe.
//Author: Andrew Olivier aolivier@ur.rochester.edu

namespace evt
{
  template <class BASE>
  class CVUniverse: public BASE
  {
    public:
      CVUniverse(const typename BASE::config_t config): BASE(config) {}
      bool IsVerticalOnly() const override { return true; }
  };
}
