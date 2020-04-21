//File: Resolution.h
//Brief: A Study template that examines correlations between a given reco variable and a truth variable.  You just need to provide
//       a VARIABLE with reco(), truth(), and name() functions.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/base/Study.h"

//utilities includes
#include "util/WithUnits.h"

#ifndef ANA_RESOLUTION_H
#define ANA_RESOLUTION_H

namespace ana
{
  template <class VARIABLE>
  class Resolution: public Study
  {
    using UNITS = decltype(std::declval<VARIABLE>().reco(std::declval<evt::CVUniverse>()));

    using HIST = PlotUtils::HistWrapper<evt::CVUniverse>;
    using HIST2D = units::WithUnits<PlotUtils::Hist2DWrapper<evt::CVUniverse>, UNITS, UNITS, events>;

    public:
      Resolution(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                     const std::vector<background_t>& backgrounds,
                     std::map<std::string, std::vector<evt::CVUniverse*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes),
                                                                                       fVar(config["variable"])
      {
        fRecoVersusTrue = dir.make<HIST2D>("Reco" + fVar.name() + "VersusTrue", "Reco " + fVar.name() + " versus True;True " + fVar.name() + ";Reco " + fVar.name(),
                                           config["binning"].as<std::vector<double>>(), config["binning"].as<std::vector<double>>(), universes);
        fResolution = dir.make<HIST>(fVar.name() + "Resolution", fVar.name() + " Resolution;#frac{Reco " + fVar.name() + " - True " + fVar.name() + "}{True " + fVar.name() + "};events",
                                     config["binning"].as<std::vector<double>>(), universes);
      }

      virtual void mcSignal(const evt::CVUniverse& event, const events weight) override
      {
        const UNITS reco = fVar.reco(event), truth = fVar.truth(event);
        fRecoVersusTrue->Fill(&event, truth, reco, weight);
        if(truth.template in<UNITS>() > 0) fResolution->FillUniverse(&event, (reco - truth).template in<UNITS>()/truth.template in<UNITS>(), weight.in<events>());
      }

      //mcBackground failed the truth signal selection.
      virtual void mcBackground(const evt::CVUniverse& /*event*/, const background_t& /*background*/, const events /*weight*/) override {}
                                                                                                                        
      //Truth tree with truth information.  Passes truth signal definition and phase space.
      virtual void truth(const evt::CVUniverse& /*event*/, const events /*weight*/) override {}
                                                                                                                        
      //Data AnaTuple with only reco information.  These events passed all reco Cuts. 
      //Truth branches may be in an undefined state here, so be very careful not to use them.
      virtual void data(const evt::CVUniverse& /*event*/) override {}
                                                                                                                        
      //Optional function called once per job after the last file in the event loop.
      //This is a good place to call syncCVHistos() or Scale() by numbers besides POT.
      //POT processed should be in the output histogram file.
      virtual void afterAllFiles(const events /*passedSelection*/) override
      {
        fRecoVersusTrue->SyncCVHistos();
        fResolution->SyncCVHistos();
      }

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

      using Registrar = ana::Study::Registrar<Resolution<VARIABLE>>;

    private:
      VARIABLE fVar;

      //Reconstructed versus truth branch in migration style
      HIST2D* fRecoVersusTrue;
      //Resolution = reco - true in case true is 0
      HIST* fResolution;
  };
}

#endif //ANA_RESOLUTION_H
