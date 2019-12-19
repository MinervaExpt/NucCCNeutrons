//File: NucCCNeutronsUnits.h
//Brief: Define base and derived units that will be used to interpret
//       the data format of NucCCNeutrons AnaTuples.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//BaseUnits includes
#include "units/units.h"

//ROOT includes
#include "Math/LorentzVector.h"

//Energy
DECLARE_UNIT(GeV)
DECLARE_RELATED_UNIT(MeV, GeV, 1, 1000)
DECLARE_RELATED_UNIT(keV, GeV, 1, 1e6)

//Distance
DECLARE_UNIT(mm)
DECLARE_RELATED_UNIT(cm, mm, 10, 1)

//Time
DECLARE_UNIT(ns)
DECLARE_RELATED_UNIT(us, ns, 1000, 1)

//Angle
DECLARE_UNIT(radians)
DECLARE_UNIT(degrees) //For now, degrees and radians aren't related

//Histogram normalizations
DECLARE_UNIT_WITH_TYPE(candidates, int)
DECLARE_UNIT_WITH_TYPE(events, int)
DECLARE_UNIT_WITH_TYPE(neutrons, int) //TODO: This might be a case where I use the full template machinery to write "FS neutrons"
DECLARE_UNIT(normalized) //For normalized histograms.

//Vertices are specified in mm
using vertex_t = ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<mm>>;
