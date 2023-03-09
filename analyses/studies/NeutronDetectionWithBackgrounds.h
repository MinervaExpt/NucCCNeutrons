//File: NeutronDetectionWithBackgrounds.h
//Brief: Reproduces Miranda and Rik's candidates per event plots from the LE neutron counting paper.
//       This is different from the NeutronDetection Study because it supports background subtraction.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"
#include "analyses/studies/CandidateMath.h"

//util includes
#include "util/Categorized.h"
#include "util/units.h"
#include "util/mathWithUnits.h"

#ifndef ANA_NEUTRONDETECTIONWITHBACKGROUNDS_H
#define ANA_NEUTRONDETECTIONWITHBACKGROUNDS_H

namespace ana
{
  class NeutronDetectionWithBackgrounds: public Study
  {
    public:
      NeutronDetectionWithBackgrounds(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                                      std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~NeutronDetectionWithBackgrounds() = default;

      //Callbacks needed for background subtraction
      virtual void data(const evt::Universe& event, const events weight) override; //Plotting in this function for both data and MC should reproduce the Miranda-and-Rik plots exactly.  Note that systematics won't work in this callback.
      //TODO: Test that the data callback reproduces my old candidates per event plots
      virtual void mcSignal(const evt::Universe& event, const events weight) override;
      virtual void mcBackground(const evt::Universe& event, const background_t& background, const events weight) override;

      virtual void afterAllFiles(const events passedSelection) override; //This is where I set the number of events (per systematic universe) to get candidates per event.  This is important for reducing uncertainties on the model prediction.

      //Callbacks I will not use
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};

      //No Truth loop needed.  Note that this disables the cut table.
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      //Cuts that decide whether a Candidate or FSPart should be counted
      ana::NeutronMultiplicity fCuts;

      //Format for neutron candidate information.
      struct NeutronCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
        int nViews;
      };

      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
        int nViews;
        int FS_index; //Mapping from a Candidate to an FSPart by index in the array of FSParts
        mm dist_to_edep_as_neutron; //Distance parent and ancestors travelled that were neutrons
      };

      //Format for FS particle information.
      struct FSPart
      {
        int PDGCode;
        MeV energy;
        double angle_wrt_z; //Angle w.r.t. the z axis of the detector in radians
        units::LorentzVector<MeV> momentum;
      };

      int encodeFSPDG(const MCCandidate& cand, const std::vector<FSPart>& fs) const;

      //Energy deposit plots for data and MC information
      using EDepHist = units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, neutrons>; //Enforce units on axes
      util::Categorized<EDepHist, int> fSignalEDeps;
      util::Categorized<util::Categorized<EDepHist, int>, background_t> fBackgroundEDeps;
      EDepHist* fDataEDeps;

      PlotUtils::HistWrapper<evt::Universe>* fNMCEntries; //Number of entries that make it into mcSignal for each universe
                                                          //This lets me cancel uncertainties when I divide by number of
                                                          //entries as well as merge playlists!
      PlotUtils::HistWrapper<evt::Universe>* fNDataEntries;
  };
}

#endif //ANA_NEUTRONDETECTIONWITHBACKGROUNDS_H
