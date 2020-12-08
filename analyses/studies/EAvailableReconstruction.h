//File: EAvailableReconstruction.h
//Brief: Study to elucidate why my available energy Cut is
//       causing efficiency > 1 in the 0 neutrons bin in
//       May 2020.  Plots available energy and neutron
//       multiplicity broken down by FS-particle-based
//       categories.  Also prints out Arachne links to
//       events that pass the reco Cut but fail the truth
//       version (and thus cause efficiency > 1).
//Author: Andrew Olivier aolivier@ur.rochester.edu

//signal includes
#include "analyses/base/Study.h"
#include "analyses/studies/NeutronMultiplicity.cpp"
#include "analyses/studies/EAvailable.cpp"

//util includes
#include "util/Categorized.h"

//c++ includes
#include <fstream>

#ifndef ANA_AVAILABLEENERGYMISRECONSTRUCTION_H
#define ANA_AVAILABLEENERGYMISRECONSTRUCTION_H

namespace ana
{
  class FSCategory;
  class NeutronMultiplicity;
  class EAvailable;

  class EAvailableReconstruction: public Study
  {
    public:
      EAvailableReconstruction(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                       std::vector<background_t>& backgrounds, std::map<std::string, std::vector<evt::Universe*>>& universes);
      virtual ~EAvailableReconstruction() = default;

      //Do this study only for MC signal events.
      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      //Normalize fPDGToObservables and syncCVHistos()
      virtual void afterAllFiles(const events /*passedSelection*/) override;

      //Do nothing for backgrounds, the Truth tree, and data
      virtual void mcBackground(const evt::Universe& /*event*/, const background_t& /*background*/, const events /*weight*/) override {};
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {};
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {}; //TODO: Do I want to plot candidate observables in data?

      //I don't need the Truth loop
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      template <class UNIT>
      using HIST = units::WithUnits<HistWrapper<evt::Universe>, UNIT, events>;

      NeutronMultiplicity fNNeutrons; //Neutron counting algorithm
      EAvailable fEAvailable; //Available energy calculator for reco and truth

      GeV fEAvailableMax; //Upper limit on available energy in both reco and truth

      std::ofstream fMismatchedCutLinks; //File of arachne links to events that pass the reco available energy Cut but
                                         //fail the truth available energy Cut.
      std::ofstream fNoRecoLinks; //File of arachne links to events with 0 reconstructed available energy

      util::Categorized<HIST<neutrons>, FSCategory*> fTruthMultiplicity; //Truth neutron multiplicity is the variable
                                                                        //in which I originally observed this problem.

      util::Categorized<HIST<unitless>, FSCategory*> fEAvailableResidual; //Truth - reco for available energy

      HIST<MeV>* fTruthAvailWhenNoReco; //Truth when there is 0 reco available energy to investigate the -1 residual peak
  };
}

#endif //ANA_AVAILABLEENERGYMISRECONSTRUCTION_H
