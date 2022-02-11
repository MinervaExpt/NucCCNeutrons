//File: FSDisappearingParticles.h
//Brief: Study FS particles that disappear before depositing all of their KE in the detector.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//base includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"

//evt includes
#include "evt/Universe.h"

//util includes
#include "util/WithUnits.h"
#include "util/units.h"
#include "util/Directory.h"
#include "util/Categorized.h"

//PlotUtils includes
//TODO: Someone who maintains this code should deal with these warnings
#pragma GCC diagnostic push //Learned to use these GCC-specific preprocessor macros from 
                            //https://stackoverflow.com/questions/6321839/how-to-disable-warnings-for-particular-include-files 
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "PlotUtils/HistWrapper.h"
#include "PlotUtils/Hist2DWrapper.h"
#pragma GCC diagnostic pop

#ifndef ANA_FSDISAPPEARINGPARTICLES_H
#define ANA_FSDISAPPEARINGPARTICLES_H

namespace
{
  const std::map<int, std::string> particles = {{-211, "piPlus"}, {211, "piMinus"}, {111, "piZero"}, {2212, "proton"}};
  const std::map<int, std::string> fates = {{0, "Stopping"}, {1, "Disappearing"}, {2, "Decay"}, {3, "Elastic Scatter"}, {4, "Inelastic Scatter"}};
}

namespace ana
{
  class FSDisappearingParticles: public Study
  {
    //First, check that VARIABLE makes sense
    private:
      using HIST = units::WithUnits<HistWrapper<evt::Universe>, MeV, events>;

    public:
      FSDisappearingParticles(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass, const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::Universe*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes),
                                                                                   fStoppingEnergiesByPDG("SignalStoppingEnergy", "True Energy", ::particles, dir,
                                                                                                          config["binning"].as<std::vector<double>>(), universes),
                                                                                   fBackgroundStoppingEnergiesByFate(backgrounds, dir, "BackgroundStoppingEnergy", "True Energy",
                                                                                                                     ::particles, dir, ::fates, dir, config["binning"].as<std::vector<double>>(), universes)
      {
      }

      virtual ~FSDisappearingParticles() = default;

      //Callbacks implemented in the separate .cpp file
      virtual void mcSignal(const evt::Universe& univ, const events weight) override;
      virtual void afterAllFiles(const events /*passedSelection*/) override;
      virtual void mcBackground(const evt::Universe& univs, const background_t& background, const events weight) override;

      //Callbacks I'm not using right now
      virtual void truth(const evt::Universe& /*univ*/, const events /*weight*/) override {}
      virtual void data(const evt::Universe& /*event*/, const events /*weight*/) override {}

    private:
      //Histogram the energies of disappearing particles as they disappear and categorize by PDG code
      util::Categorized<HIST, int> fStoppingEnergiesByPDG;
      util::Categorized<util::Categorized<util::Categorized<HIST, int>, int>, background_t> fBackgroundStoppingEnergiesByFate;
  };
}

//Register with Factory
namespace
{
  static ana::Study::Registrar<ana::FSDisappearingParticles> FSDisappeariningParticles_reg("FSDisappearingParticles");
}

#endif //ANA_FSDISAPPEARINGPARTICLES_H
