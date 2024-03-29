//File: Study.cpp
//Brief: A place to Fill() histograms with events that pass different combinations
//       of Cuts.  Each of the 4 main callback functions is called on a different
//       type of AnaTuple or with events that pass a different set of Cuts.
//       Use the Directory in the constructor to give all plots a unique name.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef SIG_SIGNAL_CPP
#define SIG_SIGNAL_CPP

//MAT includes
#include "weighters/Model.h"

//utilities includes
#include "util/Directory.h"
#include "util/Factory.cpp"
#include "util/units.h"

//yaml-cpp include for configuration
#include "yaml-cpp/yaml.h"

//c++ includes
#include <map>

namespace evt
{
  class Universe;
}

namespace ana
{
  class Background;
}

namespace reco
{
  class Cut;
}

namespace ana
{
  class Study
  {
    public:
      using cuts_t = std::vector<std::unique_ptr<reco::Cut>>;
      using background_t = std::unique_ptr<ana::Background>;

      Study(const YAML::Node& /*config*/, util::Directory& /*dir*/, cuts_t&& mustPass,
            const std::vector<background_t>& /*backgrounds*/, std::map<std::string, std::vector<evt::Universe*>>& /*universes*/);
      virtual ~Study();

      //The event loop will call these interfaces with events
      //that pass appropriate cuts.  You need to implement them in a derived class.

      //Each of these interfaces has an overload that takes multiple Universes at
      //once for very simple Studies.  When you've got a lot of IsVertical() Universes,
      //like flux Universes for example, you can Fill() them all at once and save a lot
      //of bin lookups!

      //MC AnaTuple with reco and truth information.  Passes reco selection and phase space.
      //mcSignal() passed truth signal selection too.
      virtual void mcSignal(const evt::Universe& event, const events weight) {}
      virtual void mcSignal(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt);
      
      //mcBackground failed the truth signal selection.
      virtual void mcBackground(const evt::Universe& event, const background_t& background, const events weight) {}
      virtual void mcBackground(const std::vector<evt::Universe*>& univs, const background_t& background, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt);

      //Truth tree with truth information.  Passes truth signal definition and phase space.
      virtual void truth(const evt::Universe& event, const events weight) {}
      virtual void truth(const std::vector<evt::Universe*>& univs, const PlotUtils::Model<evt::Universe>& model, const PlotUtils::detail::empty& evt);

      //Data AnaTuple with only reco information.  These events passed all reco Cuts. 
      //Truth branches may be in an undefined state here, so be very careful not to use them.
      virtual void data(const evt::Universe& event, const events weight = 1_events) = 0;

      //Optional function called once per job after the last file in the event loop.
      //This is a good place to call syncCVHistos() or Scale() by numbers besides POT.
      //POT processed should be in the output histogram file.
      virtual void afterAllFiles(const events /*passedSelection*/);

      //Function for implementing the event loop.  Do not use in derived classes.
      bool passesCuts(const evt::Universe& event);

      //Not all Studies need the Truth loop.  Override this function and return false
      //for Studies that don't need the Truth loop to more halve runtime!
      virtual bool wantsTruthLoop() const;

      //Create a static instance of an ana::Study::Registrar<> for your Study in the
      //"anonymous namespace" at the bottom of your .cpp file to make it discoverable
      //at runtime.
      template <class DERIVED>
      using Registrar =  plgn::Registrar<ana::Study, DERIVED, util::Directory&,
                                         typename ana::Study::cuts_t&&, std::vector<typename ana::Study::background_t>&,
                                         std::map<std::string, std::vector<evt::Universe*>>&>;

    private:
      //Interface for the event loop.  Behavior not guaranteed for derived plugin use!
      //Optional Cuts to define sideband samples.
      cuts_t fPasses;
  };
}

#endif //SIG_SIGNAL_CPP
