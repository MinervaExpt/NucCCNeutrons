//File: RemoveQEByCandidates.h
//Brief: Remove events with neutron candidates whose directions suggest a QE interaction. 
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef RECO_REMOVEQEBYCANDIDATES_H
#define RECO_REMOVEQEBYCANDIDATES_H

//analyses includes
#include "analyses/studies/NeutronMultiplicity.cpp"

//cut includes
#include "cuts/reco/Cut.h"

namespace evt
{
  class Universe;
}

namespace reco
{
  class RemoveQEByCandidates: public Cut
  {
    public:
      RemoveQEByCandidates(const YAML::Node& config, const std::string& name);
      virtual ~RemoveQEByCandidates() = default;

    protected:
      //Your concrete Cut class must override these methods.
      virtual bool checkCut(const evt::Universe& event, PlotUtils::detail::empty& /*empty*/) const override;

    private:
      struct NeutronCand
      {
        MeV edep;
        mm z;
        mm transverse;
      };

      ana::NeutronMultiplicity fCandSelector;

      //These 2 cosines define the region in which QE events prefer to produce candidates.
      //Cut an event whose candidates are all in this range.
      double fQECosineMin;
      double fQECosineMax;

      mm fQEDistanceMin; //Candidates must be farther than this from the vertex to be considered QE-like
      int fMaxQELikeCands; //Maximum number of candidates in the QE-rich region that will be tolerated
  };
}

#endif //RECO_REMOVEQEBYCANDIDATES_H
