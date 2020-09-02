//File: Range.h
//Brief: Cut on a Range<> in some VARIABLE.  Minimizes the number of ways I
//       can do stupid things like swapping minimum and maximum.  Also
//       an opportunity to centralize N-1 Cuts infrastructure.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_RANGE_H
#define RECO_RANGE_H

//cut includes
#include "cut/reco/Cut.h"

namespace reco
{
  template <class VARIABLE>
  class Range: public Cut
  {
    private:
      using UNIT = decltype(std::declval<VARIABLE>().reco(std::declval<evt::CVUniverse>()));

    public:
      Cut(const YAML::Node& config, const std::string& name): Cut(config, name), fVar(config["variable"]), fMin(config["min"].as<UNIT>()), fMax(config["max"].as<UNIT>())
      {
      }

      virtual ~Cut() = default;

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
      };*/

      virtual std::unique_ptr<Cut::Plotter> getPlotter(util::Directory& dir, const YAML::Node& binning, std::map<std::string, std::vector<evt::CVUniverse*>>& universes) override
      {
        return new Plotter(fVar, binning, dir, universes);
      }

    protected:
      virtual bool checkCut(const evt::CVUniverse& event, PlotUtils::detail::empty& /*empty*/) const override
      {
        const UNIT var = fVar.reco(event);
        return var >= fMin && var <= fMax;
      }

    private:
      VARIABLE fVar;
      UNIT fMin;
      UNIT fMax;
  };
}

#endif //RECO_RANGE_H
