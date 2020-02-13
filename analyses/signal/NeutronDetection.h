//File: NeutronDetection.h
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/signal/Signal.h"

//variables includes
#include "analyses/variables/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"

#ifndef SIG_NEUTRONDETECTION_H
#define SIG_NEUTRONDETECTION_H

namespace sig
{
  class NeutronDetection: public Signal
  {
    public:
      NeutronDetection(const YAML::Node& config, util::Directory& dir, std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::CVUniverse*>>& universes);
      virtual ~NeutronDetection() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const std::vector<evt::CVUniverse*>& univs) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const std::vector<evt::CVUniverse*>& /*univs*/, const background_t& /*background*/) override {};
      virtual void truth(const std::vector<evt::CVUniverse*>& /*univs*/) override {};
      virtual void data(const std::vector<evt::CVUniverse*>& /*univs*/) override {}; //TODO: Do I want to plot candidate observables in data?

    private:
      //Cuts that decide whether a Candidate or FSPart should be counted
      ana::NeutronMultiplicity fCuts;

      //Format for neutron candidate information.
      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse; //Transverse distance from the z axis of the detector
        ns time; //Time of earliest Cluster in this candidate
        int FS_index; //Mapping from a Candidate to an FSPart by index in the array of FSParts
      };

      //Format for FS particle information.
      struct FSPart
      {
        int PDGCode;
        MeV energy;
        double angle_wrt_z; //Angle w.r.t. the z axis of the detector in radians
      };

      //Histograms I'm going to Fill()
      //First, group them together by variables I'm going to histogram
      //TODO: Maybe move CandidateObservables into its own header.  That's what I eventually did last time.
      struct CandidateObservables
      {
        CandidateObservables(const std::string& name, const std::string& title, std::map<std::string, std::vector<evt::CVUniverse*>>& univs,
                             const std::vector<double>& edepBins, const std::vector<double>& angleBins, const std::vector<double>& betaBins);

        void Fill(std::map<evt::CVUniverse*, neutrons>& univs, const MCCandidate& cand, const units::LorentzVector<mm>& vertex);
        void Fill(std::map<evt::CVUniverse*, neutrons>& univ, const FSPart& fs);

        void SetDirectory(TDirectory* dir);

        units::WithUnits<HistWrapper<evt::CVUniverse>, MeV, neutrons> fEDeps;
        HistWrapper<evt::CVUniverse> fAngles;
        HistWrapper<evt::CVUniverse> fBeta;
      };

      util::Categorized<CandidateObservables, int> fPDGToObservables; //Map FS PDG code to Candidate observables
                                                                      //to explore backgrounds in phase space.
      CandidateObservables* fEffNumerator; //Neutron detection efficiency numerator
      CandidateObservables* fEffDenominator; //Neutron detection efficiency denominator
  };
}

#endif //SIG_NEUTRONDETECTION_H
