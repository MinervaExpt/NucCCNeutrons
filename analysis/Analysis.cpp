//File: Analysis.cpp
//Brief: An Analysis fills histograms with CVUniverses that meet specific criteria useful for
//       extracting a cross section.  It creates those histograms in a Directory whose name is
//       unique to that Analysis.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analysis includes
#include "analysis/Analysis.h"

//evt includes
#include "evt/CVUniverse.h"

//geo includes
#include "geo/Target.h"

//YAML-cpp includes
#include "yaml-cpp/Node.h"

namespace ana
{
  Analysis::Analysis(const YAML::Node& /*config*/, util::Directory /*dir*/, const std::vector<geo::Target>& /*truthTargets*/)
  {
  }

  //Fill signal-only plots for truth, reco, and MC which has both
  //Efficiency denominator
  void Analysis::truthSignal(const evt::CVUniverse& event)
  {
  }

  //Selected events
  void Analysis::dataSignal(const evt::CVUniverse& event)
  {
  }

  //Efficiency numerator and migration matrices
  void Analysis::MCSignal(const evt::CVUniverse& event)
  {
  }
 
  void Analysis::PlasticBackground(const evt::CVUniverse& event, const geo::Target* truthTarget)
  {
  }
 
  void Analysis::PlasticSideband(const evt::CVUniverse& event, const geo::Target* truthTarget)
  {
  }
 
  //Plots for wrong material sideband constraint.  Remember that I can just
  //use the efficiency numerators and WrongMaterialBackground()s from the
  //other sections in this target as the signal model in this sideband.
  virtual void WrongMaterialBackground(const evt::CVUniverse& event)
  {
  }
}

#endif //ANA_ANALYSIS_H
