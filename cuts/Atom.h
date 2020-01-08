//File: Atom.h
//Brief: A cut::Atom is a single node in a tree of cuts.  It reads values
//       from a CVUniverse and returns an Analysis to be filled.
//Author: Andrew Olivier aolivier@ur.rochester.edu

namespace cut
{
  class Atom
  {
    public:
      Atom(const YAML::Node& config, util::Directory& parent);
      virtual ~Atom() = default;

      virtual Analysis* truth(const CVUniverse& event) const = 0;
      virtual Analysis* reco(const CVUniverse& event) const = 0;
      virtual Analysis* mc(const CVUniverse& event) const = 0;

    protected:
      std::unique_ptr<Atom> fNext;
  };

  //TODO: These classes need their own files

  //TODO: Analysis needs to be an Atom too.  It just returns itself from truth(), reco(), and mc().

  class JustACut: public Atom
  {
    public:
      JustACut(const YAML::Node& config, util::Directory& parent);
      virtual ~JustACut() = default;

      virtual Analysis* truth(const CVUniverse& event) const override final
      {
        return isTruth(event)?fNext->truth(event):nullptr;
      }

      virtual Analysis* reco(const CVUniverse& event) const override final
      {
        return isReco(event)?fNext->reco(event):nullptr;
      }

      virtual Analysis* mc(const CVUniverse& event) const override final
      {
        if(isTruth(event) && isReco(event)) return fNext->mc(event);
        return nullptr;
      }

    protected:
      virtual bool isTruth(const CVUniverse& event) const = 0;
      virtual bool isReco(const CVUniverse& event) const = 0;
  };

  class WithSideband: public Atom
  {
    public:
      WithASideband(const YAML::Node& config, util::Directory& parent);
      virtual ~WithASideband() = default;

      virtual Analysis* truth(const CVUniverse& event) const override final
      {
        return isTruth(event)?fNext->truth(event):nullptr;
      }

      virtual Analysis* reco(const CVUniverse& event) const override final
      {
        return isReco(event)?fNext->reco(event):fSidebandTotal;
      }

      virtual Analysis* mc(const CVUniverse& event) const override final
      {
        const bool truth = isTruth(event), reco = isReco(event);
        if(reco && truth) return fNext->mc(event);
        if(reco) return fBackground;
        if(truth) return fSidebandSignal;
        return fSidebandBackground;
      }

    protected:
      virtual bool isTruth(const CVUniverse& event) const = 0;
      virtual bool isReco(const CVUniverse& event) const = 0;

    private:
      //I could either fill these in a derived class's constructor
      //or make them pure virtual functions instead.
      std::unique_ptr<Sideband> fSidebandSignal;
      std::unique_ptr<Sideband> fSidebandBackground;
      std::unique_ptr<Sideband> fSidebandTotal;
      std::unique_ptr<Background> fBackground;
  };
}
