//File: NeutronPurity.h
//Brief: This Study quantifies how well I differentiate GENIE-neutron-induced candidates from other candidates.
//       It's a good place to plot variables I'm considering adding to my candidate selection.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"
#include "util/units.h"
#include "util/mathWithUnits.h"

//c++ includes
#include <fstream>

#ifndef SIG_NEUTRONDETECTION_H
#define SIG_NEUTRONDETECTION_H

DECLARE_UNIT_WITH_TYPE_AND_YAML(Clusters, int)
DECLARE_UNIT_WITH_TYPE_AND_YAML(Digits, int)

namespace ana
{
  class NeutronPurity: public Study
  {
    public:
      NeutronPurity(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~NeutronPurity() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      //Normalize and syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      //Cuts that decide whether a Candidate or FSPart should be counted
      ana::NeutronMultiplicity fCuts;

      //Format for neutron candidate information.
      /*struct NeutronCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
        int nClusters;
      };*/ //I'd need this if I ever had something to compare to data

      struct MCCandidate
      {
        MeV edep;
        MeV caloEdep;
        mm z;
        mm transverse;
        ns time;
        int nClusters;
        int nDigits;
        MeV highestDigitE;
        int FS_index; //Mapping from a Candidate to an FSPart by index in the array of FSParts
        mm dist_to_edep_as_neutron; //Distance parent and ancestors travelled that were neutrons
        int nViews; //At least 2 views are needed to reconstruct a candidate's 3D starting position.  MINERvA has up to 3 different views.
      };

      //Format for FS particle information.
      struct FSPart
      {
        int PDGCode;
        MeV energy;
        double angle_wrt_z; //Angle w.r.t. the z axis of the detector in radians
      };

      //Helper function that needs to know about MCCandidate
      static mm distToVertex(const MCCandidate& cand, const units::LorentzVector<mm>& vertex);

      template <class XUNIT, class YUNIT, class WUNIT>
      using HIST2D = units::WithUnits<PlotUtils::Hist2DWrapper<evt::Universe>, XUNIT, YUNIT, WUNIT>;
      template <class XUNIT, class WUNIT>
      using HIST = units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, XUNIT, WUNIT>;
      using LOGHIST2D = PlotUtils::Hist2DWrapper<evt::Universe>;

      util::Categorized<HIST2D<Clusters, MeV, neutrons>, int> fPDGToEDepVersusNClusters; //Can I isolate candidates from pi0s by cutting in the energy deposited-number of Clusters plane?
      util::Categorized<HIST2D<Digits, MeV, neutrons>, int> fPDGToEDepVersusNDigits;

      util::Categorized<units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, neutrons>, int> fPDGToHighestDigitE;

      util::Categorized<HIST2D<Digits, MeV, neutrons>, int> fPDGToHighestEVersusNDigits;

      //The next 2 plots are filled with log() on each axis based on a study I did in November 2019.
      LOGHIST2D* fClosestEDepVersusDist; //Energy deposit versus distance from vertex for the closest candidate to the vertex for each FS neutron
      LOGHIST2D* fFartherEDepVersusDist; //Energy deposit versus distance from vertex for candidates that are not closest to the vertex for each FS neutron

      util::Categorized<HIST<MeV, neutrons>, int> fPDGToClosestInvMass;

      std::ofstream fSingleDigitPi0Events; //Write a file with Arachne event display links to events with 1-digit pi0-induced neutron candidates
  };
}

#endif //SIG_NEUTRONDETECTION_H
