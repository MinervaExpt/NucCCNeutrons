//File: NeutronDetection.h
//Brief: A study on how effectively I detect neutron candidates. Should plot
//       efficiency to find a FS neutron and a breakdown of fake neutron candidates
//       in multiple neutron canddiate observables.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"

//variables includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//util includes
#include "util/Categorized.h"
#include "util/units.h"
#include "util/mathWithUnits.h"

#ifndef SIG_NEUTRONDETECTION_H
#define SIG_NEUTRONDETECTION_H

namespace ana
{
  class NeutronDetection: public Study
  {
    public:
      NeutronDetection(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~NeutronDetection() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      //Normalize fPDGToObservables and syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override;

      //No Truth loop needed
      //virtual bool wantsTruthLoop() const override { return false; }

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
      };

      struct MCCandidate
      {
        MeV edep;
        mm z;
        mm transverse;
        ns time;
        int FS_index; //Mapping from a Candidate to an FSPart by index in the array of FSParts
        mm dist_to_edep_as_neutron; //Distance parent and ancestors travelled that were neutrons
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
      struct Observables
      {
        Observables(const std::string& name, const std::string& title, std::map<std::string, std::vector<evt::Universe*>>& univs,
                    const std::vector<double>& edepBins, const std::vector<double>& angleBins, const std::vector<double>& zBins,
                    const std::vector<double>& betaBins);

        template <class CANDIDATE>
        void Fill(const evt::Universe& event, const neutrons weightPerNeutron, const CANDIDATE& cand, const units::LorentzVector<mm>& vertex)
        {
          using namespace units;
          const mm deltaZ = cand.z - (vertex.z() - 17_mm); //TODO: 17mm is half a plane width.  Correction for targets?
          const mm dist = sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ));
          const double angle = deltaZ.in<mm>() / sqrt(pow<2>(cand.transverse) + pow<2>(deltaZ)).template in<mm>();
          const double beta = dist.in<mm>() / cand.time.template in<ns>() / 300.; //Speed of light is 300mm/ns
                                                                                                                        
          fEDeps.Fill(&event, cand.edep, weightPerNeutron);
          fAngles.FillUniverse(&event, angle, weightPerNeutron.in<neutrons>());
          fBeta.FillUniverse(&event, beta, weightPerNeutron.in<neutrons>());
          fZDistFromVertex.Fill(&event, cand.z - vertex.z(), weightPerNeutron);
        }

        void SetDirectory(TDirectory* dir);
        void SyncCVHistos();
        void Scale(const double value, const char* option = "");

        units::WithUnits<HistWrapper<evt::Universe>, MeV, neutrons> fEDeps;
        HistWrapper<evt::Universe> fAngles;
        HistWrapper<evt::Universe> fBeta;
        units::WithUnits<HistWrapper<evt::Universe>, mm, neutrons> fZDistFromVertex;
      };

      struct Efficiency
      {
        Efficiency(const std::string& name, const std::string& title, std::map<std::string, std::vector<evt::Universe*>>& univs,
                   const std::vector<double>& energyBins, const std::vector<double>& angleBins, const std::vector<double>& betaBins);

        void Fill(const evt::Universe& event, const neutrons weight, const FSPart& fs);

        void SetDirectory(TDirectory* dir);
        void SyncCVHistos();
        void Scale(const double value, const char* option = "");

        units::WithUnits<HistWrapper<evt::Universe>, MeV, neutrons> fEnergies;
        HistWrapper<evt::Universe> fAngles;
        HistWrapper<evt::Universe> fBeta;
      };

      util::Categorized<Observables, int> fPDGToObservables; //Map FS PDG code to Candidate observables
                                                                      //to explore backgrounds in phase space.
      Efficiency* fEffNumerator; //Neutron detection efficiency numerator
      Efficiency* fEffDenominator; //Neutron detection efficiency denominator

      Observables* fDataCands; //Neutron candidate observables in data

      units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, neutrons, events>* fCandsPerFSNeutron;
  };
}

#endif //SIG_NEUTRONDETECTION_H
