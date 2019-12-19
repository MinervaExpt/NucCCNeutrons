//File: Analysis.h
//Brief: An Analysis fills histograms with CVUniverses that meet specific criteria useful for
//       extracting a cross section.  It creates those histograms in a Directory whose name is
//       unique to that Analysis.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef ANA_ANALYSIS_H
#define ANA_ANALYSIS_H

namespace evt
{
  class CVUniverse;
}

namespace YAML
{
  class Node;
}

namespace util
{
  class Directory;
}

namespace geo
{
  class Target;
}

namespace ana
{
  class Analysis
  {
    public:
      Analysis(const YAML::Node& config, util::Directory parent, const std::vector<geo::Target>& truthTargets);
      virtual ~Analysis() = default;

      //Fill signal-only plots for truth, reco, and MC which has both
      virtual void truthSignal(const evt::CVUniverse& event); //Efficiency denominator
      virtual void dataSignal(const evt::CVUniverse& event); //Selected events
      virtual void MCSignal(const evt::CVUniverse& event); //Efficiency numerator and migration matrices

      //Plots for plastic sideband constraint
      virtual void PlasticBackground(const evt::CVUniverse& event, const geo::Target* truthTarget);
      virtual void PlasticSideband(const evt::CVUniverse& event, const geo::Target* truthTarget);

      //Plots for wrong material sideband constraint.  Remember that I can just
      //use the efficiency numerators and WrongMaterialBackground()s from the
      //other sections in this target as the signal model in this sideband.
      virtual void WrongMaterialBackground(const evt::CVUniverse& event);
  };
}

#endif //ANA_ANALYSIS_H
