add_library(studies OBJECT MuonMomentum.cpp NeutronMultiplicity.cpp NeutronDetection.cpp EAvailable.cpp EfficiencyByGENIE.cpp)
install(FILES CrossSectionSignal.h CrossSectionSideband.h NeutronDetection.h Resolution.h EfficiencyByGENIE.h DESTINATION include)
