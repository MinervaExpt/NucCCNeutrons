//File: CrossSection.h
//Brief: Background histograms needed to extract a differntial
//       cross section in some VARIABLE in 1D (TODO: more dimensions).
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef BKG_CROSSECTION_H
#define BKG_CROSSECTION_H

namespace bkg
{
  //A VARIABLE shall have:
  //1) A std::string name() const method that will be used to name all of the plots produced
  //2) A UNIT reco(const CVUniverse& univ) const method
  //3) A UNIT truth(const CVUniverse& unix) const method
  //4) The return type of reco() and truth() must match
  template <class VARIABLE>
  class CrossSection: public Background
  {
    //Sanity checks on VARIABLE
    using UNIT = decltype(fVar.reco(std::declval<evt::CVUniverse>()));
    static_assert(std::is_same<UNIT, decltype(fVar.truth(std::declval<evt::CVUniverse>()))>::value,
                  "Reco and truth variable calculations must be in the same units!");

    using HIST = HistWrapper<WithUnits<MnvH1D, UNIT, events>>;

    public:
      CrossSection(Directory& dir, cuts_t&& mustPass, const YAML::Node& config): Background(dir, mustPass, config),
                                                                                 fVar(config["variable"])
      {
        const auto binning = config["binning"].as<std::vector<double>>(); //TODO: Upgrade WithUnits<> to check UNIT on bins?
        fBackground = dir.make<HIST>("Background", ("Background;Reco " + fVar.name() + ";entries").c_str(),
                                     binning.size() - 1, binning.data());
      }

      virtual void Fill(const evt::CVUniverse& event) override
      {
        fBackground.univHist(&event).Fill(fVar.reco(event));
      }

    private:
      VARIABLE fVar;

      //This histogram will be used with the result of a sideband fit(s),
      //so it has to be in reco variables like the sideband fits.
      HIST fBackground;
  };
}

#endif //BKG_CROSSECTION_H
