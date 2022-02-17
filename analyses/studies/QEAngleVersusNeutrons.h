//File: QEAngleVersusNeutrons.h
//Brief: Plots Backgrounds for a VARIABLE with each background further broken down
//       by whether it has charged and/or neutral pions.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//studies includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//cut includes
#include "cuts/reco/Cut.h"

//evt includes
#include "evt/Universe.h"

//util includes
#include "util/Categorized.h"
#include "util/WithUnits.h"
#include "util/units.h"
#include "util/Directory.h"

//PlotUtils includes
//TODO: Someone who maintains this code should deal with these warnings
#pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                            //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#pragma GCC diagnostic pop

#ifndef ANA_QEANGLEVERSUSNETRONS_H
#define ANA_QEANGLEVERSUSNETRONS_H

namespace ana
{
  class QEAngleVersusNeutrons: public Study
  {
    public:
      QEAngleVersusNeutrons(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                               std::vector<background_t>& backgrounds,
                               std::map<std::string, std::vector<evt::Universe*>>& univs);

      virtual ~QEAngleVersusNeutrons() = default;

      virtual void mcSignal(const evt::Universe& event, const events weight) override;

      virtual void mcBackground(const evt::Universe& event, const background_t& background, const events weight) override;

      virtual void afterAllFiles(const events /*passedSelection*/) override;

      //Functions I don't plan to use
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {}

      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {}

    private:
      ana::NeutronMultiplicity fNeutronCounter;

      //Validation histograms for the kinematic calculations I'm relying on
      units::WithUnits<PlotUtils::HistWrapper<evt::Universe>, MeV, events>* fENuQEResolution;
      PlotUtils::HistWrapper<evt::Universe>* fMissingPCosineResolution;

      //The main result: absolute difference between neutron direction and QE hypothesis missing momentum
      PlotUtils::HistWrapper<evt::Universe>* fSignalNeutronAngleDiff;
      util::Categorized<PlotUtils::HistWrapper<evt::Universe>, background_t> fBackgroundNeutronAngleDiff;

      //My selection has mulitple candidates.  Maybe I can do better by cutting on the closest candidate to the expected QE direction?
      //The closest candidate to the vertex?
      PlotUtils::HistWrapper<evt::Universe>* fSignalBestNeutronAngleDiff;
      util::Categorized<PlotUtils::HistWrapper<evt::Universe>, background_t> fBackgroundBestNeutronAngleDiff;

      MeV calcENuQE(const evt::Universe& univ) const;
  };
}

#endif //ANA_QEANGLEVERSUSNETRONS_H
