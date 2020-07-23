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

#ifndef SIG_NEUTRONDETECTION_H
#define SIG_NEUTRONDETECTION_H

DECLARE_UNIT_WITH_TYPE_AND_YAML(Clusters, int)

namespace ana
{
  class NeutronPurity: public Study
  {
    public:
      NeutronPurity(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::CVUniverse*>>& universes);
      virtual ~NeutronPurity() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override;

      //Normalize and syncCVHistos()
      virtual void afterAllFiles(const events passedSelection) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::CVUniverse& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::CVUniverse& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::CVUniverse& /*event*/) override {};

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

      util::Categorized<units::WithUnits<PlotUtils::Hist2DWrapper<evt::CVUniverse>, Clusters, MeV, neutrons>, int> fPDGToEDepVersusNClusters; //Can I isolate candidates from pi0s by cutting in the energy deposited-number of Clusters plane?
  };
}

#endif //SIG_NEUTRONDETECTION_H
