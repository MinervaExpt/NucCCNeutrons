//File: Fiducial.h
//Brief: A Fiducial volume is a geometric region of MINERvA that a neutrino
//       interaction vertex has to be in to be counted for a cross section.
//       Each Fiducial has its own set of Cuts to produce its own cut summary.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "analyses/base/Study.h"
#include "analyses/base/Background.h"
#include "PlotUtils/Cut.h"
#include "PlotUtils/Cutter.h"
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"
#include "util/Factory.cpp"
#include "evt/Universe.h"

//c++ includes
#include <vector>
#include <unordered_map>
#include <memory>

namespace fid
{
  //A Target contains one or more Fiducials
  /*class Target
  {
    public:
      virtual std::unique_ptr<Fiducial>& findRecoSection(const evt::Universe& univ) = 0;
      virtual std::unique_ptr<Fiducial>& findTruthSection(const evt::Universe& univ) = 0;
  };*/

  //TODO: Rename Fiducial as Section?
  class Fiducial
  {
    public:
      Fiducial(const YAML::Node& /*config*/) {}
               
      virtual ~Fiducial() = default;

      //virtual double NNucleons(const bool isMC) const = 0;
      virtual PlotUtils::MnvH1D* NNucleons(const bool isMC) const = 0;

      std::string name;
      std::vector<PlotUtils::Cut<evt::Universe>*> recoCuts;
      std::vector<PlotUtils::SignalConstraint<evt::Universe>*> signalDef;
      std::vector<PlotUtils::SignalConstraint<evt::Universe>*> phaseSpace;

      std::unique_ptr<ana::Study> study;
      std::unique_ptr<PlotUtils::Cutter<evt::Universe>> selection;
      std::unordered_map<std::bitset<64>, std::vector<std::unique_ptr<ana::Study>>> sidebands;
      std::vector<std::unique_ptr<ana::Background>> backgrounds;

      template <class DERIVED>
      using Registrar = plgn::Registrar<fid::Fiducial, DERIVED>;
  };
}
