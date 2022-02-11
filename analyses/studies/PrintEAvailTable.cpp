//File: PrintEAvailTable.h
//Brief: MINERvA has a web app called Arachne for its sole event display.  This program spits out
//       links to Arachne pages on std::clog == STDERR.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//analyses includes
#include "analyses/base/Study.h"
#include "analyses/base/Background.h"
#include "analyses/studies/EAvailable.cpp"
#include "analyses/studies/NeutronMultiplicity.cpp"

//utilities includes
#include "util/WithUnits.h"
#include "util/SafeROOTName.h"

//evt includes
#include "evt/Universe.h"
#include "evt/arachne.h"
#include "evt/EventID.h"

//MAT includes
#include "PlotUtils/Table.h"

//c++ includes
#include <iostream>
#include <fstream>
#include <unordered_map>

#ifndef ANA_EVENTDISPLAY_H
#define ANA_EVENTDISPLAY_H

namespace
{
  const std::array<std::string, 9> columnNames = {"Run", "Subrun", "Event", "Slice", "Reco EAvail [MeV]", "Reco OD Energy [MeV]", "Reco Summed Neutron Energy [MeV]", "N Tracks", "Arachne Link"};
}

namespace ana
{
  class PrintEAvailTable: public Study
  {
    public:
      PrintEAvailTable(const YAML::Node& config, util::Directory& dir, cuts_t&& mustPass,
                   const std::vector<background_t>& backgrounds,
                   std::map<std::string, std::vector<evt::Universe*>>& universes): Study(config, dir, std::move(mustPass), backgrounds, universes),
                   fSignalTable(::columnNames), fSelectedTable(::columnNames), fEAvail(config["EAvailable"]), fNNeutrons(config["neutronCounter"])
      {
        for(const auto& background: backgrounds)
        {
          fBackgroundTables.emplace(background.get(), ::columnNames);
        }
        fBackgroundTables.emplace(nullptr, ::columnNames); //nullptr is the "Other" background category, designed to be used with Categorized
      }

      virtual void mcSignal(const evt::Universe& event, const events /*weight*/) override
      {
        const auto eventID = event.GetEventID(false);
        auto neutronCands = event.Get<NeutronMultiplicity::Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
        const auto vertex = event.GetVtx();

        MeV neutronEDep = 0_MeV;
        for(const auto& cand: neutronCands)
        {
          if(fNNeutrons.countAsReco(cand, vertex)) neutronEDep += cand.edep;
        }

        fSignalTable.appendRow(eventID.run, eventID.subrun, eventID.gate, eventID.slice,
                               fEAvail.reco(event).in<MeV>(), event.GetODEnergy().in<MeV>(),
                               neutronEDep.in<MeV>(), event.GetNTracks(), util::arachne(eventID, false));
      }

      //mcBackground failed the truth signal selection.
      virtual void mcBackground(const evt::Universe& event, const background_t& background, const events /*weight*/) override
      {
        const auto eventID = event.GetEventID(false);
        auto neutronCands = event.Get<NeutronMultiplicity::Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
        const auto vertex = event.GetVtx();

        MeV neutronEDep = 0_MeV;
        for(const auto& cand: neutronCands)
        {
          if(fNNeutrons.countAsReco(cand, vertex)) neutronEDep += cand.edep;
        }

        fBackgroundTables.at(background.get()).appendRow(eventID.run, eventID.subrun, eventID.gate, eventID.slice,
                                                         fEAvail.reco(event).in<MeV>(), event.GetODEnergy().in<MeV>(),
                                                         neutronEDep.in<MeV>(), event.GetNTracks(), util::arachne(eventID, false));
      }
                                                                                                                        
      //TODO: If truth-only studies ever work, I need to do some printing here
      //Truth tree with truth information.  Passes truth signal definition and phase space.
      virtual void truth(const evt::Universe& /*event*/, const events /*weight*/) override {}
                                                                                                                        
      //Data AnaTuple with only reco information.  These events passed all reco Cuts. 
      //Truth branches may be in an undefined state here, so be very careful not to use them.
      virtual void data(const evt::Universe& event, const events /*weight*/) override
      {
        const auto eventID = event.GetEventID(true);
        auto neutronCands = event.Get<NeutronMultiplicity::Candidate>(event.Getblob_edep(), event.Getblob_zPos(), event.Getblob_transverse_dist_from_vertex());
        const auto vertex = event.GetVtx();

        MeV neutronEDep = 0_MeV;
        for(const auto& cand: neutronCands)
        {
          if(fNNeutrons.countAsReco(cand, vertex)) neutronEDep += cand.edep;
        }

        fSelectedTable.appendRow(eventID.run, eventID.subrun, eventID.gate, eventID.slice,
                                 fEAvail.reco(event).in<MeV>(), event.GetODEnergy().in<MeV>(),
                                 neutronEDep.in<MeV>(), event.GetNTracks(), util::arachne(eventID, true));
      }
                                                                                                                        
      //Optional function called once per job after the last file in the event loop.
      virtual void afterAllFiles(const events /*passedSelection*/) override
      {
        std::ofstream signalFile("signalTable.txt");
        fSignalTable.print(signalFile);

        std::ofstream selectedFile("selectedTable.txt");
        fSelectedTable.print(selectedFile);

        for(const auto& background: fBackgroundTables)
        {
          const auto name = background.first?background.first->name():"Other";
          std::ofstream backgroundTable(util::SafeROOTName(name) + "Table.txt");
          background.second.print(backgroundTable);
        }
      }

      //No Truth loop needed
      virtual bool wantsTruthLoop() const override { return false; }

    private:
      util::Table<9> fSignalTable;
      util::Table<9> fSelectedTable;
      std::unordered_map<ana::Background*, util::Table<9>> fBackgroundTables;
      EAvailable fEAvail;
      NeutronMultiplicity fNNeutrons;
  };
}

namespace
{
  static ana::Study::Registrar<ana::PrintEAvailTable> PrintEAvailTable_reg("PrintEAvailTable");
}

#endif //ANA_EVENTDISPLAY_H
