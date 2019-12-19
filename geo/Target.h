//File: Target.h
//Brief: A Target decides whether a vertex is in a slice of the detector in z.
//       It is made up of one or more material sections.
//Author: Andrew Olivier aolivier@ur.rochester.edu

/*
//Example of how Target will be used in event loops
std::map<std::unique_ptr<Section>, std::unique_ptr<Analysis>> sectionToAnalysis;

//Truth tree
{
  for(auto& target: targets)
  {
    if(target->isInside(truthEvent))
    {
      sectionToAnalysis[target->section(truthEvent)].Signal(truthEvent); //Fill efficiency denominator
    }
  }
}

//Data tree
{
  //TODO: Each section within a target is a sideband for the other sections in that target.
  //      Can I just use other signals from the same target when it's time for the background constraint fit?
  const auto target = std::find_if(targets.begin(), targets.end(), [&recoEvent](const auto& target) { return target.isInside(recoEvent); });
  if(target != targets.end() && target->distanceToDivision(recoEvent)) target->section(recoEvent).Signal(recoEvent); //Fill selection histogram
  else //This event is in the plastic sideband
  {
    const auto plastic = std::find_if(plasticSlices.begin(), plasticSlices.end(), [&recoEvent](const auto& slice) { return slice.isInside(recoEvent); });
    if(plastic != plasticSlices.end()) //Then this is a plastic sideband event
    {
      for(auto& target: plastic->adjacent()) //There could be 1 or 2 adjacent targets
      {
        if(target->distanceToDivision(recoEvent)) sectionToAnalysis[target->section(recoEvent)].PlasticSideband(recoEvent);
      }
    }
  }
}

//MC tree
{
  const auto target = std::find_if(targets.begin(), targets.end(), [&event](const auto& target) { return target.isInside((RecoCVUniverse&)event); });
  if(target != targets.end())
  {
    if(target->isInside((TruthCVUniverse&)MCEvent) && target->distanceToDivision(MCEvent))
    {
      auto& section = target->section((RecoCVUniverse&)MCEvent);
      //TODO: Each section within a target is a sideband for the other sections in that target.  So, I always want my sideband plots for signal events.
      if(section.isTruthSignal(MCEvent)) sectionToAnalysis[section].Signal(MCEvent); //Signal, so fill migration and efficiency numerator
      else sectionToAnalysis[section].WrongMaterialBackground(MCEvent); //TODO: Break down by true material name?  That's a study, but it does need to be in a paper
                                                     //      that uses this sideband.  I also want the total.
    }
    else //This is a plastic background event.  Fill plastic background plots
    {
      sectionToAnalysis[target->section((RecoCVUniverse&)MCEvent)].PlasticBackground(MCEvent); //TODO: Break down by true target/plastic name?  That's a study, but it does need to be in a paper
                                                  //      that uses this sideband.
    }
  }
  else //This is probably a plastic sideband event
  {
    const auto plastic = std::find_if(plasticSlices.begin(), plasticSlices.end(), [&MCEvent](const auto& slice) { return slice.isInside(MCEvent); });
    if(plastic != plasticSlices.end()) //Then this is a plastic sideband event
    {
      for(auto& target: plastic->adjacent()) //There could be 1 or 2 adjacent targets
      {
        if(target->distanceToDivision(MCEvent)) sectionToAnalysis[target->section((RecoCVUniverse&)MCEvent)].PlasticSideband(MCEvent); //TODO: Break down by true target/plastic name?  That's a study, but it
                                                                            //      does need to be in a paper that uses this sideband.
      }
    }
  }
}

//TODO: What does a study look like?  Seems like I can at least use the same functions.
//      I'll probably need an "MC that just passes truth cuts" loop.
*/

namespace ana
{
  class Analysis
  {
    public:
      Analysis(Directory& dir);
      virtual ~Analysis() = default;

      //Fill signal-only plots for truth, reco, and MC which has both
      virtual void Signal(const TruthCVUniverse& event); //Efficiency denominator
      virtual void Signal(const RecoCVUniverse& event); //Selected events
      virtual void Signal(const MCCVUniverse& event); //Efficiency numerator and migration matrices

      //Plots for plastic sideband constraint
      virtual void PlasticBackground(const MCCVUniverse& event);

      virtual void PlasticSideband(const MCCVUniverse& event);
      virtual void PlasticSideband(const RecoCVUniverse& event);

      //Plots for wrong material sideband constraint.  Remember that I can just
      //use the efficiency numerators and WrongMaterialBackground()s from the
      //other sections in this target as the signal model in this sideband.
      virtual void WrongMaterialBackground(const MCCVUniverse& event);

    private:
      //TODO: Plots and categories I'm going to fill.
  };

  //TODO: I could generalize this to take any variable.  See my CrossSection class template in the current version of plotting.
  class CrossSection: public Analysis
  {
    public:
      CrossSection(Directory& dir);
      virtual ~CrossSection() = default;

      //Fill signal-only plots for truth, reco, and MC which has both
      virtual void Signal(const TruthCVUniverse& event) override; //Efficiency denominator
      virtual void Signal(const RecoCVUniverse& event) override; //Selected events
      virtual void Signal(const MCCVUniverse& event) override; //Efficiency numerator and migration matrices

      //Plots for plastic sideband constraint
      virtual void PlasticBackground(const MCCVUniverse& event) override;

      virtual void PlasticSideband(const MCCVUniverse& event) override;
      virtual void PlasticSideband(const RecoCVUniverse& event) override;

      //Plots for wrong material sideband constraint.  Remember that I can just
      //use the efficiency numerators and WrongMaterialBackground()s from the
      //other sections in this target as the signal model in this sideband.
      virtual void WrongMaterialBackground(const MCCVUniverse& event) override;

    private:
      struct Sideband
      {
        Sideband(Directory& dir);
        MnvH1D* Background;
        MnvH1D* Sideband;
      };

      std::unordered_map<Section*, Sideband> fTruthSectionToSideband; //Mapping from truth Section to sideband plots
      std::unordered_map<int, MnvH1D*> fMaterialToSideband; //The wrong material sideband.  Materials are specified by Z.
      MnvH2D* fMigration;
      MnvH1D* fSignalEvents;
      MnvH1D* fEfficiencyNum;
      MnvH1D* fEfficiencyDenom;
  };
}

namespace geo
{
  class Section
  {
    public:
      Section(const YAML::Node& config, Directory& dir);
      virtual ~Section() = default;

      virtual bool isTruth(const TruthCVUniverse& event) const; //Is event's truth record inside this section?
  };

  //TODO: Implement specific targets
  class Target
  {
    public:
      //TODO: Either Target and Section need to create Analyses, or they do not need a Directory.
      Target(const YAML::Node& config, Directory& dir);
      virtual ~Target() = default;

      virtual bool isInside(const TruthCVUniverse& event); //Truth only
      virtual bool isInside(const RecoCVUniverse& event); //Reco

      virtual bool distanceToDivision(const RecoCVUniverse& event);

      virtual Section& section(const TruthCVUniverse& event); //Map (x, y) to a material section in this Target
      virtual Section& section(const RecoCVUniverse& event);

    private:
      mm fZMin; //Beginning z position of this Target
      mm fZMax; //End z position of this Target

      std::vector<std::unique_ptr<Section>> fSections;
  };

  class PlasticSlice
  {
    public:
      PlasticSlice(const YAML::Node& config);
      virtual ~PlasticSlice() = default;

      //Check whether an event is inside the PlasticSlice
      bool isInside(const RecoCVUniverse& event) const;

      //Access to adjacent targets
      const std::vector<Target*>& adjacent();

    private:
      std::vector<Target*> fAdjacent;

      mm fZMin; //Beginning z position of this slice of plastic
      mm fZMax; //End z position of this slice of plastic
  };
}
