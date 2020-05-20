//File: UpperLimit.h
//Brief: Cut on a UpperLimit<> in some VARIABLE.  Minimizes the number of ways I
//       can do stupid things like swapping minimum and maximum.  Also
//       an opportunity to centralize N-1 Cuts infrastructure.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_UPPERLIMIT_H
#define RECO_UPPERLIMIT_H

//cut includes
#include "cuts/reco/Cut.h"

namespace reco
{
  template <class UNIT, UNIT(evt::CVUniverse::*reco)() const>
  class UpperLimit: public Cut
  {
    public:
      UpperLimit(const YAML::Node& config, const std::string& name): Cut(config, name), fMax(config["max"].as<UNIT>())
      {
      }

      virtual ~UpperLimit() = default;

      //Sketch of N-1 Cuts infrastructure
      /*
      class Plotter: public Cut::Plotter
      {
        private:
          using HIST = units::WithUnits<HistWrapper<evt::CVUniverse>, UNIT, events>;

        public:
          Plotter(VARIABLE& var, const YAML::Node& binning, util::Directory& dir, std::map<std::string, std::vector<evt::CVUniverse*>>& universes): fVar(var)
          {
            fCutValues = dir.make<HIST>(util::safeROOTName(fVar.name()), fVar.name() + ";Reco " + fVar.name() + ";", binning.as<std::vector<double>>());
          }

          virtual void Fill(const evt::CVUniverse& event, const events weight) override
          {
            fCutValues->Fill(&event, fVar.reco(event));
          }

          virtual void afterAllEvents() override
          {
            fCutValues->SyncCVHistos();
          }

        private:
          VARIABLE fVar;

          HIST* fCutValues; //Value this Cut would have selected on
      };

      virtual std::unique_ptr<Cut::Plotter> getPlotter(util::Directory& dir, const YAML::Node& binning, std::map<std::string, std::vector<evt::CVUniverse*>>& universes) override
      {
        return new Plotter(fVar, binning, dir, universes);
      }*/

    protected:
      virtual bool passesCut(const evt::CVUniverse& event) const override
      {
        return (event.*reco)() <= fMax;
      }

    private:
      UNIT fMax;
  };
}

#endif //RECO_UPPERLIMIT_H
